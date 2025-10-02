#!/usr/bin/env python3

from setuptools import setup, find_packages
import os

# Get version from CMake
version = "1.0.0"

setup(
    name="asfm-logger",
    version=version,
    description="ASFMLogger Python Wrapper - Enterprise-Grade Logging Framework",
    long_description="""ASFMLogger (Abstract Shared File Map Logger) is a comprehensive, enterprise-grade, multi-language logging framework built on advanced architectural principles.

This Python wrapper provides native Python interface to the enhanced ASFMLogger C++ library with features like:
- Multi-instance logging with application tracking
- Smart message classification with importance framework
- Contextual persistence with adaptive policies
- Intelligent queue management with priority preservation
- SQL Server integration for enterprise database logging
- Cross-platform shared memory support
""",
    long_description_content_type="text/markdown",
    author="ASFMLogger Development Team",
    author_email="support@asfmlogger.dev",
    url="https://github.com/AlotfyDev/ASFMLogger",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: System :: Logging",
    ],
    python_requires=">=3.7",
    install_requires=[
        # No external dependencies required for basic functionality
    ],
    extras_require={
        "dev": [
            "pytest>=6.0",
            "pytest-cov",
            "black",
            "flake8",
            "mypy",
        ],
        "sql": [
            "pyodbc",  # For SQL Server support
        ],
    },
    package_data={
        "asfm_logger": ["*.py", "*.md"],
    },
    include_package_data=True,
    zip_safe=False,
    entry_points={
        "console_scripts": [
            "asfm-logger-config=asfm_logger.cli:main",
        ],
    },
    keywords=[
        "logging", "enterprise", "multi-language", "c++", "performance",
        "sql-server", "shared-memory", "real-time", "monitoring"
    ],
)
