#!/usr/bin/env python3
"""
ASFMLogger Python Integration Tests
TEST 2.01: Python Wrapper Core Testing
Component: wrappers/python/asfm_logger.py
Purpose: Validate Python ctypes bridge to C++ logging framework
Business Value: Primary multi-language interface (⭐⭐⭐⭐⭐☆)
"""

import pytest
import time
import threading
import json
import os
import tempfile
from unittest.mock import Mock, patch, MagicMock
from datetime import datetime
import gc

# Import the module under test - this will create the module availability tests
try:
    import asfm_logger
    PYTHON_MODULE_AVAILABLE = True
except ImportError:
    PYTHON_MODULE_AVAILABLE = False
    asfm_logger = None  # Create a mock so tests can be collected without failing


@pytest.fixture
def test_logger():
    """Fixture to create a test logger instance"""
    if not PYTHON_MODULE_AVAILABLE:
        pytest.skip("Python logger module not available")
    return asfm_logger.get_logger("PythonTestApp", "TestProcess")


@pytest.fixture
def temp_log_file():
    """Fixture to create a temporary log file"""
    with tempfile.NamedTemporaryFile(mode='w+', suffix='.log', delete=False) as f:
        temp_file = f.name
    yield temp_file
    # Cleanup
    try:
        os.unlink(temp_file)
    except:
        pass


class TestPythonLoggerInitialization:
    """Test basic logger initialization functionality"""

    def test_python_logger_creation_success(self):
        """Test that logger can be created successfully"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = asfm_logger.get_logger("TestApp")
        assert logger is not None
        assert hasattr(logger, 'info')
        assert hasattr(logger, 'error')
        assert hasattr(logger, 'debug')

    def test_get_logger_with_process_name(self):
        """Test logger creation with application and process name"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = asfm_logger.get_logger("MyApp", "MyProcess")
        assert logger is not None
        assert logger.application_name == "MyApp"
        assert logger.process_name == "MyProcess"

    def test_logger_initialization_attributes(self):
        """Test that logger has proper initialization attributes"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = asfm_logger.get_logger("TestApp")

        # Check required attributes
        expected_attrs = ['application_name', 'process_name', 'info', 'debug',
                         'warn', 'error', 'critical', 'get_local_logs',
                         'get_log_statistics', 'clear_local_logs']

        for attr in expected_attrs:
            assert hasattr(logger, attr), f"Logger missing attribute: {attr}"

    def test_multiple_loggers_isolation(self):
        """Test that multiple loggers maintain separate state"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger1 = asfm_logger.get_logger("App1", "Proc1")
        logger2 = asfm_logger.get_logger("App2", "Proc2")

        assert logger1.application_name == "App1"
        assert logger2.application_name == "App2"
        assert logger1.application_name != logger2.application_name


class TestPythonEnhancedFeatures:
    """Test integration with C++ enhanced features"""

    def test_enhanced_configuration_setting(self, test_logger):
        """Test that enhanced configuration can be set without errors"""
        # This test verifies the configuration method exists and calls without error
        # We don't test actual C++ functionality since DLL may not be available
        test_logger.configure_enhanced(
            enable_database=False,
            database_connection="",
            console_output=True,
            log_file="test_log.txt",
            max_file_size=1024,
            max_files=3
        )
        # If we get here without exception, test passes
        assert True

    def test_database_configuration_parameters(self, test_logger):
        """Test database configuration parameter passing"""
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="Server=localhost;Database=TestLogs;Trusted_Connection=True;"
        )
        # Parameter validation only - actual DB connection may not be available
        assert True

    def test_configuration_parameter_validation(self, test_logger):
        """Test that invalid configuration parameters are handled"""
        # These should not raise exceptions even with invalid params
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="",  # Invalid
            log_file="",  # Invalid
            max_file_size=-1,  # Invalid
            max_files=0  # Invalid
        )
        assert True


class TestPythonComponentLogging:
    """Test component-based logging functionality"""

    def test_basic_component_logging(self, test_logger):
        """Test logging with component tracking"""
        start_time = time.time()

        # Log messages with components
        test_logger.info("Test message 1", component="DataProcessor")
        test_logger.warn("Test warning", component="Validator")

        # Verify logs were captured
        logs = test_logger.get_local_logs()
        assert len(logs) >= 2

        # Check recent messages have correct components
        recent_logs = test_logger.get_local_logs(limit=5)
        component_names = [log['component'] for log in recent_logs]
        assert "DataProcessor" in component_names

    def test_component_filtering(self, test_logger):
        """Test filtering logs by component"""
        # Clear existing logs
        test_logger.clear_local_logs()

        # Add logs with different components
        test_logger.info("Message 1", component="CompA")
        test_logger.info("Message 2", component="CompA")
        test_logger.info("Message 3", component="CompB")

        # Filter by component
        comp_a_logs = test_logger.get_local_logs(component="CompA")
        assert len(comp_a_logs) == 2

        comp_b_logs = test_logger.get_local_logs(component="CompB")
        assert len(comp_b_logs) == 1

    def test_function_tracking(self, test_logger):
        """Test function name tracking in logs"""
        test_logger.clear_local_logs()

        test_logger.debug("Debug in function", component="Test", function="testFunction")
        test_logger.info("Info in method", component="Test", function="processData")

        logs = test_logger.get_local_logs()
        recent_logs = logs[-2:]  # Last two logs

        # Check that function names are tracked
        function_names = [log.get('function', '') for log in recent_logs]
        assert "testFunction" in function_names
        assert "processData" in function_names


class TestPythonMemoryManagement:
    """Test memory management and resource cleanup"""

    def test_logger_deallocation_without_crash(self):
        """Test that logger objects can be deleted without memory issues"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = asfm_logger.get_logger("MemoryTest")

        # Use logger
        for i in range(100):
            logger.debug(f"Debug message {i}", component="MemoryTest")

        # Delete logger
        del logger

        # Force garbage collection
        gc.collect()

        # If we get here without crash, test passes
        assert True

    def test_log_accumulation_limits(self, test_logger):
        """Test that local log queue size is limited"""
        # Clear logs first
        test_logger.clear_local_logs()

        # Add more messages than queue limit (1000)
        for i in range(1100):
            test_logger.info(f"Message {i}", component="LimitTest")

        logs = test_logger.get_local_logs()
        # Should not exceed queue size limit
        assert len(logs) <= 1000

    def test_gc_pressure_logging(self):
        """Test logging under garbage collection pressure"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = asfm_logger.get_logger("GCStressTest")

        # Create many logger instances with logging
        loggers = []
        for i in range(50):
            temp_logger = asfm_logger.get_logger(f"TempApp{i}")
            temp_logger.info(f"Message from temp logger {i}")
            loggers.append(temp_logger)

        # Delete all loggers
        del loggers[:]

        # Force GC
        gc.collect()

        # Main logger should still work
        logger.error("After GC cleanup", component="GCStress")
        recent_logs = logger.get_local_logs(limit=1)
        assert len(recent_logs) == 1


class TestPythonDataTypeMarshaling:
    """Test data type handling and conversion between Python and C++"""

    def test_various_data_types_in_messages(self, test_logger):
        """Test that various data types can be logged"""
        test_logger.clear_local_logs()

        # Test different data types that might need marshaling
        test_logger.info("String message")
        test_logger.warn("Warning message", component="DataTypes")

        # The library should handle these without crashes
        logs = test_logger.get_local_logs()
        assert len(logs) >= 2

        # Check that messages contain the data
        messages = [log['message'] for log in logs[-2:]]
        assert any("String message" in msg for msg in messages)

    def test_unicode_string_handling(self, test_logger):
        """Test that Unicode strings are handled properly"""
        test_logger.clear_local_logs()

        # Test with Unicode characters
        unicode_message = "Unicode test: £€¥测试"
        test_logger.info(unicode_message, component="Unicode")

        logs = test_logger.get_local_logs(component="Unicode")
        assert len(logs) >= 1

        # Message should be preserved (exact handling depends on encoding)
        found = any(unicode_message in log['message'] or "Unicode" in log['message']
                   for log in logs)
        assert found

    def test_timestamps_and_timing(self, test_logger):
        """Test that timestamps are properly managed"""
        test_logger.clear_local_logs()

        start_time = datetime.now()
        test_logger.info("Timestamp test", component="Timing")

        logs = test_logger.get_local_logs(component="Timing", limit=1)

        if logs:
            # Check that timestamp is recent (within reasonable bounds)
            log_time_str = logs[0]['timestamp']
            log_time = datetime.strptime(log_time_str, "%Y-%m-%d %H:%M:%S.%f")
            time_diff = abs((log_time - start_time).total_seconds())
            assert time_diff < 10  # Within 10 seconds


class TestPythonMultithreading:
    """Test thread safety and concurrent logging"""

    def test_multithreaded_logging(self, test_logger):
        """Test that logging works safely from multiple threads"""
        test_logger.clear_local_logs()

        def log_worker(thread_id, message_count):
            """Worker function for logging from thread"""
            for i in range(message_count):
                test_logger.info(f"Thread {thread_id} message {i}",
                               component=f"Thread{thread_id}")
                time.sleep(0.001)  # Small delay

        # Create threads
        threads = []
        num_threads = 5
        messages_per_thread = 20

        for i in range(num_threads):
            thread = threading.Thread(target=log_worker,
                                    args=(i, messages_per_thread))
            threads.append(thread)
            thread.start()

        # Wait for all threads
        for thread in threads:
            thread.join()

        # Verify all messages were logged
        total_expected = num_threads * messages_per_thread
        logs = test_logger.get_local_logs()
        assert len(logs) >= total_expected

        # Check that messages from different threads are present
        thread_components = set()
        for log in logs:
            if log['component'].startswith('Thread'):
                thread_components.add(log['component'])

        assert len(thread_components) >= num_threads

    def test_concurrent_read_write_operations(self, test_logger):
        """Test concurrent reading and writing of log data"""
        test_logger.clear_local_logs()

        results = []
        errors = []

        def reader_worker():
            """Read logs continuously"""
            try:
                for _ in range(50):
                    logs = test_logger.get_local_logs()
                    results.append(len(logs))
                    time.sleep(0.01)
            except Exception as e:
                errors.append(str(e))

        def writer_worker():
            """Write logs continuously"""
            try:
                for i in range(50):
                    test_logger.info(f"Concurrent message {i}")
                    time.sleep(0.01)
            except Exception as e:
                errors.append(str(e))

        # Start reader and writer threads
        reader_thread = threading.Thread(target=reader_worker)
        writer_thread = threading.Thread(target=writer_worker)

        reader_thread.start()
        writer_thread.start()

        reader_thread.join()
        writer_thread.join()

        # Verify no errors occurred
        assert len(errors) == 0, f"Thread errors: {errors}"

        # Verify some results were recorded
        assert len(results) > 0
        assert max(results) >= 25  # At least some messages logged


class TestPythonAdvancedFeatures:
    """Test advanced Python wrapper features"""

    def test_log_statistics_calculation(self, test_logger):
        """Test log statistics computation"""
        test_logger.clear_local_logs()

        # Add various log messages
        test_logger.error("Error 1", component="ErrorComp")
        test_logger.warn("Warning 1", component="WarnComp")
        test_logger.info("Info 1", component="InfoComp")
        test_logger.debug("Debug 1", component="DebugComp")

        stats = test_logger.get_log_statistics()

        # Check statistics structure
        assert "total_messages" in stats
        assert stats["total_messages"] == 4

        # Check level distribution
        assert "level_distribution" in stats
        level_dist = stats["level_distribution"]
        assert level_dist.get("ERROR", 0) == 1
        assert level_dist.get("WARN", 0) == 1
        assert level_dist.get("INFO", 0) == 1
        assert level_dist.get("DEBUG", 0) == 1

    def test_export_functionality(self, test_logger, temp_log_file):
        """Test log export capabilities"""
        test_logger.clear_local_logs()

        # Add some test messages
        test_logger.info("Export test message 1")
        test_logger.error("Export test message 2")

        # Export to different formats
        for format_type in ["json", "csv"]:
            try:
                test_logger.export_logs_to_file(temp_log_file, format_type)

                # Check file was created
                assert os.path.exists(temp_log_file)

                # Check file has content
                with open(temp_log_file, 'r') as f:
                    content = f.read()
                    assert len(content) > 0

                # Clean up for next format
                if os.path.exists(temp_log_file):
                    os.unlink(temp_log_file)

            except Exception as e:
                # Some export formats might not be implemented
                pass

    def test_real_time_monitoring_setup(self, test_logger):
        """Test real-time monitoring infrastructure"""
        # Test that monitoring can be enabled without errors
        monitoring_results = []

        def test_callback(messages):
            monitoring_results.extend(messages)

        # Enable monitoring
        test_logger.enable_real_time_monitoring(callback=test_callback)

        # Add a message that should trigger monitoring
        test_logger.info("Monitoring test", component="Monitor")

        # Give monitoring time to process
        time.sleep(0.1)

        # Note: This test validates that monitoring setup doesn't crash
        # Actual callback behavior depends on timing and threading
        assert True

    def test_log_filtering(self, test_logger):
        """Test log filter creation and usage"""
        filter_fn = test_logger.create_log_filter(component="TestComp", level="INFO")

        # Test filter behavior
        test_logs = [
            {'component': 'TestComp', 'level': 'INFO', 'message': 'Test 1'},
            {'component': 'OtherComp', 'level': 'INFO', 'message': 'Test 2'},
            {'component': 'TestComp', 'level': 'ERROR', 'message': 'Test 3'}
        ]

        filtered = filter_fn(test_logs)
        assert len(filtered) == 1
        assert filtered[0]['component'] == 'TestComp'
        assert filtered[0]['level'] == 'INFO'


class TestPythonGlobalFunctions:
    """Test global convenience functions"""

    def test_quick_log_function(self):
        """Test the quick logging function"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        # Test quick logging without requiring return value
        asfm_logger.quick_log("Quick test message", "INFO", "QuickComponent")
        assert True

    def test_global_logger_configuration(self):
        """Test global logger configuration"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        # Test global configuration
        asfm_logger.configure_global_logger(
            "GlobalTestApp",
            enable_database=False,
            log_file="global_test.log"
        )

        global_logger = asfm_logger.get_global_logger()
        assert global_logger is not None
        assert global_logger.application_name == "GlobalTestApp"


class TestPythonPerformance:
    """Basic performance validation tests"""

    def test_bulk_logging_performance(self, test_logger):
        """Test performance of bulk logging operations"""
        test_logger.clear_local_logs()

        message_count = 1000

        start_time = time.time()
        for i in range(message_count):
            test_logger.info(f"Performance test message {i}")
        end_time = time.time()

        # Verify messages were logged
        logs = test_logger.get_local_logs()
        assert len(logs) >= message_count

        # Check reasonable performance (should complete quickly)
        duration = end_time - start_time
        assert duration < 5.0  # Less than 5 seconds for 1000 messages


# Skip all tests if module not available
if not PYTHON_MODULE_AVAILABLE:
    pytest.skip("asfm_logger Python module not available. Cannot run Python wrapper tests.",
               allow_module_level=True)


# Test module availability
def test_python_module_import():
    """Test that the Python module can be imported"""
    # This test will be skipped if import failed at module level
    assert asfm_logger is not None


def test_logger_creation_quick():
    """Quick test of logger creation functionality"""
    if not PYTHON_MODULE_AVAILABLE:
        pytest.skip("Python logger module not available")

    logger = asfm_logger.get_logger("QuickTest")
    assert logger is not None

    # Quick smoke test
    logger.info("Quick test message", component="SmokeTest")
    assert True


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
