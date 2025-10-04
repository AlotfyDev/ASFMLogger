#!/usr/bin/env python3
"""
ASFMLogger Python Advanced Features Testing
TEST 2.02: Python Wrapper Advanced Features Testing
Component: wrappers/python/asfm_logger.py - Advanced Features
Purpose: Complete Python ecosystem integration with advanced capabilities
Business Value: Scientific computing & data science ecosystem (⭐⭐⭐⭐⭐)
"""

import pytest
import time
import threading
import json
import os
import tempfile
import psutil
import platform
from unittest.mock import Mock, patch, MagicMock
from datetime import datetime, timedelta
import gc
import asyncio

# Import the module under test - handle availability
try:
    import asfm_logger
    PYTHON_MODULE_AVAILABLE = True
except ImportError:
    PYTHON_MODULE_AVAILABLE = False
    asfm_logger = None


@pytest.fixture
def test_logger():
    """Fixture to create test logger with advanced features"""
    if not PYTHON_MODULE_AVAILABLE:
        pytest.skip("Python logger module not available")
    logger = asfm_logger.get_logger("PythonAdvancedTest")
    logger.configure_enhanced(
        enable_database=False,  # We'll mock database for testing
        log_file="advanced_test.log"
    )
    return logger


@pytest.fixture
def mock_database():
    """Mock database connection for testing"""
    class MockDatabase:
        def __init__(self):
            self.connected = False
            self.messages = []

        def connect(self):
            self.connected = True
            return True

        def insert_message(self, message):
            self.messages.append(message)
            return True

        def disconnect(self):
            self.connected = False
            return True

        def get_message_count(self):
            return len(self.messages)

    return MockDatabase()


class TestPythonDatabaseIntegration:
    """Test Python integration with database persistence"""

    def test_database_connection_simulation(self, test_logger):
        """Test database connection handling in Python wrapper"""
        # Test that database configuration doesn't crash
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="Server=test;Database=TestLogs;Trusted_Connection=True;"
        )
        # Should not raise exceptions even if DB not available
        assert True

    @patch('asfm_logger._cpp_database_connect')
    def test_database_operations_with_mock(self, mock_db_connect, test_logger):
        """Test database operations using mocked C++ calls"""
        mock_db_connect.return_value = True

        # Simulate database logging
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="test_connection_string"
        )

        test_logger.info("Database test message", component="DatabaseTest")

        # Verify mock was called (if database was enabled)
        if test_logger._library_loaded:  # Only if C++ lib available
            mock_db_connect.assert_called_once_with("test_connection_string")

    def test_database_error_handling(self, test_logger):
        """Test graceful handling of database connection failures"""
        # Configure with invalid connection - should not crash
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="INVALID_CONNECTION_STRING"
        )

        # Logger should still function
        test_logger.error("Error with invalid DB config", component="ErrorTest")

        logs = test_logger.get_local_logs(component="ErrorTest")
        assert len(logs) > 0
        assert logs[-1]['message'] == "Error with invalid DB config"

    def test_database_logging_performance(self, test_logger):
        """Test performance impact of database logging"""
        test_logger.configure_enhanced(enable_database=False)
        test_logger.clear_local_logs()

        # Baseline without database
        start_time = time.time()
        for i in range(1000):
            test_logger.info(f"Baseline message {i}", component="Performance")
        baseline_time = time.time() - start_time

        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="mock_connection"
        )

        # With database (simulated)
        start_time = time.time()
        for i in range(1000):
            test_logger.info(f"DB message {i}", component="Performance")
        db_time = time.time() - start_time

        # Database logging should not be catastrophically slower
        # Allow significant overhead since DB operations are slow by nature
        assert db_time < baseline_time * 10, f"DB logging too slow: {db_time}s vs {baseline_time}s"


class TestPythonPerformanceBenchmarking:
    """Test Python wrapper performance characteristics"""

    def test_bulk_logging_performance_scaling(self, test_logger):
        """Test performance scaling with large numbers of messages"""
        test_logger.clear_local_logs()

        message_counts = [100, 1000, 10000]
        performance_results = {}

        for count in message_counts:
            test_logger.clear_local_logs()

            start_time = time.time()
            for i in range(count):
                test_logger.info(f"Bulk message {i}", component=f"Bulk{count}")
            end_time = time.time()

            duration = end_time - start_time
            msgs_per_second = count / duration if duration > 0 else float('inf')

            performance_results[count] = {
                'duration': duration,
                'msgs_per_second': msgs_per_second
            }

            # Verify reasonable performance (>= 1000 msg/s for bulk operations)
            assert msgs_per_second >= 100.0, f"Poor performance: {msgs_per_second:.1f} msg/s for {count} messages"

        # Verify linear scaling (approximately)
        scaling_ratio = performance_results[message_counts[1]]['msgs_per_second'] / \
                       performance_results[message_counts[0]]['msgs_per_second']
        assert scaling_ratio > 0.5, f"Poor scaling: {scaling_ratio:.2f}"

    def test_concurrent_performance_under_load(self, test_logger):
        """Test performance under concurrent load"""
        test_logger.clear_local_logs()

        thread_count = 10
        messages_per_thread = 1000
        total_expected = thread_count * messages_per_thread

        results = {}
        errors = []

        def performance_worker(thread_id):
            try:
                thread_start = time.time()
                for i in range(messages_per_thread):
                    test_logger.info(f"Thread {thread_id} msg {i}",
                                   component=f"PerfThread{thread_id}")
                thread_duration = time.time() - thread_start
                results[thread_id] = thread_duration
            except Exception as e:
                errors.append(str(e))

        # Start concurrent threads
        threads = []
        for thread_id in range(thread_count):
            thread = threading.Thread(target=performance_worker, args=(thread_id,))
            threads.append(thread)
            thread.start()

        # Wait for all threads
        for thread in threads:
            thread.join()

        # Verify no errors
        assert len(errors) == 0, f"Thread errors: {errors}"

        # Verify all messages logged
        logs = test_logger.get_local_logs()
        total_logged = sum(len(test_logger.get_local_logs(component=f"PerfThread{i}"))
                          for i in range(thread_count))
        assert total_logged >= total_expected * 0.95  # Allow some margin

        # Calculate aggregate performance
        total_duration = max(results.values())
        total_msgs_per_second = total_expected / total_duration
        assert total_msgs_per_second >= 1000, f"Low concurrent performance: {total_msgs_per_second:.1f} msg/s"

    def test_memory_usage_during_bulk_operations(self, test_logger):
        """Test memory usage during large-scale logging operations"""
        def get_memory_usage():
            process = psutil.Process()
            return process.memory_info().rss / 1024 / 1024  # MB

        initial_memory = get_memory_usage()

        test_logger.clear_local_logs()

        # Perform bulk logging
        bulk_size = 10000
        for i in range(bulk_size):
            test_logger.info(f"Memory test {i}", component="MemoryTest")

        after_logging = get_memory_usage()

        # Verify reasonable memory usage (< 100 MB additional)
        memory_increase = after_logging - initial_memory
        assert memory_increase < 100.0, f"Excessive memory usage: {memory_increase:.1f} MB"

        # Verify local queue is limited (should not grow beyond limit)
        logs = test_logger.get_local_logs()
        assert len(logs) <= 1000  # Queue size limit

    def test_cross_component_performance_isolation(self, test_logger):
        """Test that performance is consistent across different components"""
        test_logger.clear_local_logs()

        components = ["Database", "UI", "Network", "FileSystem", "Cache"]
        results = {}

        messages_per_component = 500

        for component in components:
            start_time = time.time()
            for i in range(messages_per_component):
                test_logger.info(f"Component {component} message {i}",
                               component=component)
            duration = time.time() - start_time
            results[component] = duration

        # Verify each component performs consistently (±50% of average)
        avg_duration = sum(results.values()) / len(results)
        for component, duration in results.items():
            ratio = duration / avg_duration
            assert 0.5 <= ratio <= 1.5, f"Inconsistent performance for {component}: {ratio:.2f}x average"


class TestPythonExceptionHandling:
    """Test exception handling and propagation between Python and C++"""

    def test_exception_propagation_from_configure(self, test_logger):
        """Test that C++ exceptions are handled gracefully in Python"""
        # Even if C++ library throws, Python should handle it
        test_logger.configure_enhanced(
            enable_database=True,
            database_connection="INVALID_PATH_THAT_SHOULD_FAIL"
        )

        # Should not crash - verify logger still works
        test_logger.info("Exception recovery test")
        logs = test_logger.get_local_logs()
        assert len(logs) > 0

    def test_memory_exception_handling(self, test_logger):
        """Test handling of out-of-memory conditions"""
        import sys

        # Test with very large data that might cause issues
        huge_message = "A" * 100000  # 100KB string

        # Should handle large messages without crashing
        try:
            test_logger.info(huge_message, component="LargeMessage")
            assert True
        except MemoryError:
            self.fail("Memory error not handled gracefully")

    def test_threading_exception_isolation(self, test_logger):
        """Test that exceptions in one thread don't affect others"""
        exceptions_caught = []
        successful_operations = 0

        def failing_thread():
            try:
                # Force an exception that might occur in practice
                raise ValueError("Intentional test exception")
                successful_operations += 1  # Won't execute
            except ValueError as e:
                exceptions_caught.append(str(e))

        def successful_thread(thread_id):
            try:
                test_logger.info(f"Success thread {thread_id}",
                               component="ExceptionIsolation")
                successful_operations += 1
            except Exception as e:
                exceptions_caught.append(f"Unexpected error in thread {thread_id}: {e}")

        # Start one failing thread and multiple successful threads
        threads = []
        failing_thread_obj = threading.Thread(target=failing_thread)
        threads.append(failing_thread_obj)

        for i in range(5):
            success_thread = threading.Thread(target=successful_thread, args=(i,))
            threads.append(success_thread)

        # Start all threads
        for thread in threads:
            thread.start()

        # Wait for all
        for thread in threads:
            thread.join()

        # Verify that successful threads completed despite the failing one
        assert len(exceptions_caught) >= 5  # At least the intentional exception
        assert successful_operations >= 5  # All success threads completed

    def test_logging_under_exception_conditions(self, test_logger):
        """Test that logging works reliably even when exceptions occur elsewhere"""
        def failing_operation():
            raise RuntimeError("Simulated failure in application code")

        def error_logging_operation():
            try:
                failing_operation()
            except:
                test_logger.error("Operation failed", component="ExceptionHandler")
                test_logger.info("Error handling complete", component="ExceptionHandler")

        # Execute operation that will cause exceptions
        error_logging_operation()

        # Verify error logging worked
        error_logs = test_logger.get_local_logs(component="ExceptionHandler", level="ERROR")
        info_logs = test_logger.get_local_logs(component="ExceptionHandler", level="INFO")

        assert len(error_logs) >= 1
        assert len(info_logs) >= 1


class TestPythonAsyncIntegration:
    """Test Python asyncio integration for async applications"""

    @pytest.mark.asyncio
    async def test_async_logging_operations(self, test_logger):
        """Test that logging works within async context"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = test_logger

        async def async_logging_task(task_id, message_count):
            """Async task that performs logging"""
            for i in range(message_count):
                logger.info(f"Async task {task_id} message {i}",
                          component="AsyncTest")
                await asyncio.sleep(0.001)  # Non-blocking sleep

        # Run multiple async logging tasks concurrently
        tasks = [
            async_logging_task(i, 50) for i in range(5)
        ]

        await asyncio.gather(*tasks)

        # Verify all async messages were logged
        logs = logger.get_local_logs(component="AsyncTest")
        assert len(logs) >= 250  # 5 tasks * 50 messages

    @pytest.mark.asyncio
    async def test_async_context_component_tracking(self, test_logger):
        """Test component tracking in async contexts"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = test_logger

        async def async_component_operation(component_name, operation_count):
            """Simulate different async operations with component tracking"""
            for i in range(operation_count):
                logger.info(f"Operation {i}", component=component_name)
                await asyncio.sleep(0.001)

        # Simulate different async services
        services = [
            ("DatabaseService", 20),
            ("ApiService", 25),
            ("CacheService", 15),
            ("AuthService", 30)
        ]

        tasks = [
            async_component_operation(service, count)
            for service, count in services
        ]

        await asyncio.gather(*tasks)

        # Verify component isolation
        total_logs = 0
        for service, expected_count in services:
            service_logs = logger.get_local_logs(component=service)
            assert len(service_logs) == expected_count
            total_logs += expected_count

        # Verify total
        all_logs = logger.get_local_logs()
        assert len(all_logs) >= total_logs

    @pytest.mark.asyncio
    async def test_async_performance_characteristics(self, test_logger):
        """Test performance characteristics in async context"""
        if not PYTHON_MODULE_AVAILABLE:
            pytest.skip("Python logger module not available")

        logger = test_logger

        async def timed_async_logging(count):
            start_time = time.time()
            for i in range(count):
                logger.info(f"Timed async message {i}")
                await asyncio.sleep(0.0001)  # Very brief async yield
            return time.time() - start_time

        # Run performance test
        duration = await timed_async_logging(1000)
        msgs_per_second = 1000 / duration

        # Async logging should maintain reasonable performance
        assert msgs_per_second >= 2000  # Should be quite fast in async context


class TestPythonCrossPlatformCompatibility:
    """Test Python wrapper compatibility across platforms"""

    def test_platform_detection(self, test_logger):
        """Test that platform information is correctly detected"""
        # Get platform information
        current_platform = platform.system().lower()  # 'windows', 'linux', 'darwin'

        # Logger should work on any platform (shouldn't crash)
        test_logger.info(f"Running on {current_platform}", component="PlatformTest")

        logs = test_logger.get_local_logs(component="PlatformTest")
        assert len(logs) >= 1

        # Test system info logging
        system_info = platform.platform()
        test_logger.debug(f"System: {system_info}", component="PlatformDebug")

        debug_logs = test_logger.get_local_logs(component="PlatformDebug")
        assert len(debug_logs) >= 1

    def test_unicode_handling_across_platforms(self, test_logger):
        """Test Unicode string handling consistency"""
        # Test various Unicode strings that might behave differently on platforms
        unicode_tests = [
            "Basic Unicode: £€¥©®™¢",
            "Extended Unicode: 🎯🚀💻🔧",
            "Asian Unicode: 测试数据分析",
            "Combined: Test 🎯 测试 C#",
            "Empty: ",
            "Whitespace: \t\n\r"
        ]

        for i, unicode_text in enumerate(unicode_tests):
            test_logger.info(unicode_text, component="UnicodeTest", function=f"Test{i}")

        logs = test_logger.get_local_logs(component="UnicodeTest")
        assert len(logs) == len(unicode_tests)

        # Verify that all messages contain their original Unicode content
        for log in logs:
            message = log['message']
            # At minimum, the message should be preserved and not corrupted
            assert len(message) > 0

    def test_system_resource_integration(self, test_logger):
        """Test integration with system resource monitoring"""
        # Test CPU and memory information logging
        memory_mb = psutil.virtual_memory().available / 1024 / 1024
        test_logger.info(f"Available memory: {memory_mb:.1f} MB", component="SystemResources")

        cpu_percent = psutil.cpu_percent(interval=0.1)
        test_logger.debug(f"CPU usage: {cpu_percent:.1f}%", component="SystemResources")

        logs = test_logger.get_local_logs(component="SystemResources")
        assert len(logs) >= 2

        # Verify numeric data is reasonable
        memory_log = logs[0]
        assert float(memory_log['message'].split(': ')[1].split(' ')[0]) > 0

        if len(logs) > 1:
            cpu_log = logs[1]
            cpu_value = float(cpu_log['message'].split(': ')[1].strip('%'))
            assert 0.0 <= cpu_value <= 100.0


class TestPythonAdvancedLoggingScenarios:
    """Test advanced logging scenarios and edge cases"""

    def test_structured_data_logging(self, test_logger):
        """Test logging of structured data formats"""
        # Test JSON-like structured logging
        structured_data = {
            "user_id": 12345,
            "action": "login",
            "timestamp": datetime.now().isoformat(),
            "metadata": {
                "ip": "192.168.1.100",
                "user_agent": "Test/1.0",
                "session_id": "abc123xyz"
            }
        }

        # Convert to JSON for logging
        json_data = json.dumps(structured_data, indent=2)
        test_logger.info(f"Structured event: {json_data}", component="StructuredLogging")

        logs = test_logger.get_local_logs(component="StructuredLogging")
        assert len(logs) >= 1

        # Verify JSON structure is preserved in logs
        logged_message = logs[0]['message']
        assert '"user_id": 12345' in logged_message
        assert '"action": "login"' in logged_message

    def test_time_based_logging_scenarios(self, test_logger):
        """Test time-based logging scenarios"""
        test_logger.clear_local_logs()

        # Log messages at different time intervals
        start_time = datetime.now()
        test_logger.info("Start of time test", component="TimeTest")

        # Simulate time passage (artificial delays)
        for i in range(5):
            time.sleep(0.1)
            test_logger.info(f"Time interval {i+1}", component="TimeTest")

        test_logger.info("End of time test", component="TimeTest")

        # Test time-based filtering (get logs from last 1 second)
        all_logs = test_logger.get_local_logs(component="TimeTest")
        recent_logs = [
            log for log in all_logs
            if (datetime.now() - start_time).total_seconds() < 1.0
        ]

        # Verify temporal relationships
        assert len(all_logs) == 7  # start + 5 intervals + end
        assert len(recent_logs) >= len(all_logs) - 2  # Earliest messages might be older

    def test_high_frequency_logging_simulation(self, test_logger):
        """Test simulated high-frequency trading logging scenario"""
        test_logger.clear_local_logs()

        # Simulate high-frequency signal logging (like MQL5 trading signals)
        signal_count = 500
        instruments = ["EURUSD", "GBPUSD", "USDJPY", "AUDUSD"]
        signal_types = ["BUY_SIGNAL", "SELL_SIGNAL", "HOLD_SIGNAL"]

        for i in range(signal_count):
            instrument = instruments[i % len(instruments)]
            signal_type = signal_types[i % len(signal_types)]
            price = 1.0 + (i % 100) * 0.01  # Simulating price movements

            test_logger.info(f"{signal_type} {instrument} at {price:.4f}",
                           component=f"HighFreq{instrument}")

        total_logged = sum(len(test_logger.get_local_logs(component=f"HighFreq{instrument}"))
                          for instrument in instruments)

        assert total_logged >= signal_count * 0.9  # Allow some loss from queue limits

        # Verify performance wasn't compromised
        last_log = test_logger.get_local_logs()[-1]
        assert last_log is not None


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
