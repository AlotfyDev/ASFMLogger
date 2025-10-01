// ASFMLogger Dashboard JavaScript

class ASFMLoggerDashboard {
    constructor() {
        this.apiBaseUrl = 'http://localhost:8080/api';
        this.wsUrl = 'ws://localhost:8080/ws';
        this.websocket = null;
        this.isConnected = false;
        this.autoRefreshInterval = null;
        this.isAutoRefreshEnabled = true;
        this.currentFilters = {
            application: '',
            component: '',
            level: 'INFO',
            timeRange: '24'
        };

        // Charts
        this.messageRateChart = null;
        this.errorDistributionChart = null;
        this.applicationActivityChart = null;
        this.systemHealthChart = null;

        // Data storage
        this.logMessages = [];
        this.applications = [];
        this.components = [];
        this.statistics = {};

        this.initialize();
    }

    async initialize() {
        console.log('Initializing ASFMLogger Dashboard...');

        // Setup event listeners
        this.setupEventListeners();

        // Initialize charts
        this.initializeCharts();

        // Connect to WebSocket
        this.connectWebSocket();

        // Load initial data
        await this.loadInitialData();

        // Start auto-refresh
        this.startAutoRefresh();

        console.log('Dashboard initialized successfully');
    }

    setupEventListeners() {
        // Filter controls
        document.getElementById('applicationFilter').addEventListener('change', (e) => {
            this.currentFilters.application = e.target.value;
            this.applyFilters();
        });

        document.getElementById('componentFilter').addEventListener('change', (e) => {
            this.currentFilters.component = e.target.value;
            this.applyFilters();
        });

        document.getElementById('levelFilter').addEventListener('change', (e) => {
            this.currentFilters.level = e.target.value;
            this.applyFilters();
        });

        document.getElementById('timeRange').addEventListener('change', (e) => {
            this.currentFilters.timeRange = e.target.value;
            this.refreshData();
        });

        // Control buttons
        document.getElementById('refreshBtn').addEventListener('click', () => {
            this.refreshData();
        });

        document.getElementById('clearLogsBtn').addEventListener('click', () => {
            this.clearLogDisplay();
        });

        document.getElementById('exportLogsBtn').addEventListener('click', () => {
            this.exportLogs();
        });

        document.getElementById('toggleAutoRefresh').addEventListener('click', () => {
            this.toggleAutoRefresh();
        });

        document.getElementById('pauseStreamBtn').addEventListener('click', () => {
            this.toggleStream();
        });

        document.getElementById('clearStreamBtn').addEventListener('click', () => {
            this.clearStream();
        });

        // Search functionality
        document.getElementById('searchBtn').addEventListener('click', () => {
            this.performSearch();
        });

        document.getElementById('searchInput').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                this.performSearch();
            }
        });

        document.getElementById('advancedSearchBtn').addEventListener('click', () => {
            this.showAdvancedSearch();
        });

        // Modal functionality
        document.querySelector('.close-modal').addEventListener('click', () => {
            this.hideAdvancedSearch();
        });

        document.getElementById('modalSearchBtn').addEventListener('click', () => {
            this.performAdvancedSearch();
        });

        // Close modal when clicking outside
        window.addEventListener('click', (e) => {
            const modal = document.getElementById('advancedSearchModal');
            if (e.target === modal) {
                this.hideAdvancedSearch();
            }
        });
    }

    initializeCharts() {
        const chartConfig = {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'top',
                },
            },
        };

        // Message Rate Chart
        const messageRateCtx = document.getElementById('messageRateChart').getContext('2d');
        this.messageRateChart = new Chart(messageRateCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Messages per Minute',
                    data: [],
                    borderColor: 'rgb(59, 130, 246)',
                    backgroundColor: 'rgba(59, 130, 246, 0.1)',
                    tension: 0.1
                }]
            },
            options: chartConfig
        });

        // Error Distribution Chart
        const errorDistCtx = document.getElementById('errorDistributionChart').getContext('2d');
        this.errorDistributionChart = new Chart(errorDistCtx, {
            type: 'doughnut',
            data: {
                labels: ['TRACE', 'DEBUG', 'INFO', 'WARN', 'ERROR', 'CRITICAL'],
                datasets: [{
                    data: [0, 0, 0, 0, 0, 0],
                    backgroundColor: [
                        '#6b7280',
                        '#3b82f6',
                        '#10b981',
                        '#f59e0b',
                        '#ef4444',
                        '#dc2626'
                    ]
                }]
            },
            options: chartConfig
        });

        // Application Activity Chart
        const appActivityCtx = document.getElementById('applicationActivityChart').getContext('2d');
        this.applicationActivityChart = new Chart(appActivityCtx, {
            type: 'bar',
            data: {
                labels: [],
                datasets: [{
                    label: 'Active Instances',
                    data: [],
                    backgroundColor: 'rgba(16, 185, 129, 0.8)',
                    borderColor: 'rgb(16, 185, 129)',
                    borderWidth: 1
                }]
            },
            options: chartConfig
        });

        // System Health Chart
        const systemHealthCtx = document.getElementById('systemHealthChart').getContext('2d');
        this.systemHealthChart = new Chart(systemHealthCtx, {
            type: 'gauge',
            data: {
                datasets: [{
                    data: [0, 25, 50, 75, 100],
                    value: 0,
                    backgroundColor: ['#dc2626', '#f59e0b', '#eab308', '#84cc16', '#10b981'],
                    borderWidth: 0
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: false }
                }
            }
        });
    }

    connectWebSocket() {
        try {
            this.websocket = new WebSocket(this.wsUrl);

            this.websocket.onopen = () => {
                console.log('WebSocket connected');
                this.updateConnectionStatus(true);
                this.subscribeToLogs();
            };

            this.websocket.onmessage = (event) => {
                this.handleWebSocketMessage(event.data);
            };

            this.websocket.onclose = () => {
                console.log('WebSocket disconnected');
                this.updateConnectionStatus(false);
                // Attempt to reconnect after 5 seconds
                setTimeout(() => this.connectWebSocket(), 5000);
            };

            this.websocket.onerror = (error) => {
                console.error('WebSocket error:', error);
                this.updateConnectionStatus(false);
            };

        } catch (error) {
            console.error('Failed to connect WebSocket:', error);
            this.updateConnectionStatus(false);
        }
    }

    updateConnectionStatus(connected) {
        const statusIndicator = document.querySelector('.status-indicator');
        const statusText = document.querySelector('.status-text');

        if (connected) {
            statusIndicator.className = 'status-indicator status-connected';
            statusText.textContent = 'Connected';
            this.isConnected = true;
        } else {
            statusIndicator.className = 'status-indicator status-disconnected';
            statusText.textContent = 'Disconnected';
            this.isConnected = false;
        }
    }

    subscribeToLogs() {
        if (this.websocket && this.websocket.readyState === WebSocket.OPEN) {
            const subscription = {
                type: 'subscribe',
                filters: this.currentFilters
            };
            this.websocket.send(JSON.stringify(subscription));
        }
    }

    handleWebSocketMessage(data) {
        try {
            const message = JSON.parse(data);

            switch (message.type) {
                case 'log_message':
                    this.addLogMessage(message.data);
                    break;
                case 'statistics':
                    this.updateStatistics(message.data);
                    break;
                case 'system_health':
                    this.updateSystemHealth(message.data);
                    break;
                case 'applications':
                    this.updateApplicationList(message.data);
                    break;
                case 'components':
                    this.updateComponentList(message.data);
                    break;
            }
        } catch (error) {
            console.error('Error handling WebSocket message:', error);
        }
    }

    async loadInitialData() {
        try {
            // Load applications
            await this.loadApplications();

            // Load components
            await this.loadComponents();

            // Load initial statistics
            await this.loadStatistics();

            // Load recent logs
            await this.loadRecentLogs();

        } catch (error) {
            console.error('Error loading initial data:', error);
        }
    }

    async loadApplications() {
        try {
            const response = await fetch(`${this.apiBaseUrl}/applications`);
            const data = await response.json();

            this.applications = data.applications || [];
            this.updateApplicationFilter();
        } catch (error) {
            console.error('Error loading applications:', error);
        }
    }

    async loadComponents() {
        try {
            const response = await fetch(`${this.apiBaseUrl}/components`);
            const data = await response.json();

            this.components = data.components || [];
            this.updateComponentFilter();
        } catch (error) {
            console.error('Error loading components:', error);
        }
    }

    async loadStatistics() {
        try {
            const response = await fetch(`${this.apiBaseUrl}/statistics?hours=${this.currentFilters.timeRange}`);
            const data = await response.json();

            this.statistics = data;
            this.updateStatisticsDisplay();
            this.updateCharts();
        } catch (error) {
            console.error('Error loading statistics:', error);
        }
    }

    async loadRecentLogs() {
        try {
            const params = new URLSearchParams({
                limit: '100',
                hours: this.currentFilters.timeRange
            });

            if (this.currentFilters.application) {
                params.append('application', this.currentFilters.application);
            }

            const response = await fetch(`${this.apiBaseUrl}/logs?${params}`);
            const data = await response.json();

            this.logMessages = data.logs || [];
            this.updateLogDisplay();
        } catch (error) {
            console.error('Error loading recent logs:', error);
        }
    }

    updateApplicationFilter() {
        const select = document.getElementById('applicationFilter');
        select.innerHTML = '<option value="">All Applications</option>';

        this.applications.forEach(app => {
            const option = document.createElement('option');
            option.value = app.name;
            option.textContent = `${app.name} (${app.instance_count} instances)`;
            select.appendChild(option);
        });
    }

    updateComponentFilter() {
        const select = document.getElementById('componentFilter');
        select.innerHTML = '<option value="">All Components</option>';

        this.components.forEach(component => {
            const option = document.createElement('option');
            option.value = component;
            option.textContent = component;
            select.appendChild(option);
        });
    }

    updateStatisticsDisplay() {
        document.getElementById('totalMessages').textContent = this.statistics.total_messages || 0;
        document.getElementById('errorCount').textContent = this.statistics.total_errors || 0;
        document.getElementById('activeApps').textContent = this.statistics.active_applications || 0;
        document.getElementById('queueSize').textContent = this.statistics.queue_size || 0;
    }

    updateLogDisplay() {
        const logStream = document.getElementById('logStream');
        logStream.innerHTML = '';

        if (this.logMessages.length === 0) {
            logStream.innerHTML = '<div class="stream-placeholder"><p>No log messages to display</p></div>';
            return;
        }

        this.logMessages.forEach((log, index) => {
            const logEntry = document.createElement('div');
            logEntry.className = `log-entry ${log.level.toLowerCase()}`;
            logEntry.innerHTML = `
                <strong>[${log.timestamp}]</strong>
                <span class="level">[${log.level}]</span>
                <span class="component">[${log.component}]</span>
                <span class="message">${this.escapeHtml(log.message)}</span>
            `;
            logStream.appendChild(logEntry);
        });

        // Scroll to bottom
        logStream.scrollTop = logStream.scrollHeight;
    }

    addLogMessage(logMessage) {
        // Add to local storage
        this.logMessages.unshift(logMessage);

        // Keep only last 1000 messages
        if (this.logMessages.length > 1000) {
            this.logMessages = this.logMessages.slice(0, 1000);
        }

        // Update display if filters match
        if (this.messageMatchesFilters(logMessage)) {
            this.addLogEntryToDisplay(logMessage);
        }

        // Update statistics
        this.updateStatisticsDisplay();
    }

    addLogEntryToDisplay(logMessage) {
        const logStream = document.getElementById('logStream');

        // Remove placeholder if present
        const placeholder = logStream.querySelector('.stream-placeholder');
        if (placeholder) {
            placeholder.remove();
        }

        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${logMessage.level.toLowerCase()} new`;
        logEntry.innerHTML = `
            <strong>[${logMessage.timestamp}]</strong>
            <span class="level">[${logMessage.level}]</span>
            <span class="component">[${logMessage.component}]</span>
            <span class="message">${this.escapeHtml(logMessage.message)}</span>
        `;

        logStream.insertBefore(logEntry, logStream.firstChild);

        // Keep only last 200 messages in display
        while (logStream.children.length > 200) {
            logStream.lastChild.remove();
        }

        // Scroll to top for new messages
        logStream.scrollTop = 0;
    }

    messageMatchesFilters(message) {
        if (this.currentFilters.application && message.application !== this.currentFilters.application) {
            return false;
        }
        if (this.currentFilters.component && message.component !== this.currentFilters.component) {
            return false;
        }
        if (this.currentFilters.level && this.getLevelPriority(message.level) < this.getLevelPriority(this.currentFilters.level)) {
            return false;
        }
        return true;
    }

    getLevelPriority(level) {
        const priorities = {
            'TRACE': 0,
            'DEBUG': 1,
            'INFO': 2,
            'WARN': 3,
            'ERROR': 4,
            'CRITICAL': 5
        };
        return priorities[level] || 0;
    }

    updateCharts() {
        this.updateMessageRateChart();
        this.updateErrorDistributionChart();
        this.updateApplicationActivityChart();
        this.updateSystemHealthChart();
    }

    updateMessageRateChart() {
        // This would be updated with real data from the API
        const labels = [];
        const data = [];

        for (let i = 59; i >= 0; i--) {
            const time = new Date(Date.now() - i * 60000);
            labels.push(time.toLocaleTimeString());
            data.push(Math.floor(Math.random() * 100)); // Mock data
        }

        this.messageRateChart.data.labels = labels;
        this.messageRateChart.data.datasets[0].data = data;
        this.messageRateChart.update();
    }

    updateErrorDistributionChart() {
        const levelCounts = {
            'TRACE': 0,
            'DEBUG': 0,
            'INFO': 0,
            'WARN': 0,
            'ERROR': 0,
            'CRITICAL': 0
        };

        this.logMessages.forEach(log => {
            if (levelCounts.hasOwnProperty(log.level)) {
                levelCounts[log.level]++;
            }
        });

        this.errorDistributionChart.data.datasets[0].data = [
            levelCounts.TRACE,
            levelCounts.DEBUG,
            levelCounts.INFO,
            levelCounts.WARN,
            levelCounts.ERROR,
            levelCounts.CRITICAL
        ];
        this.errorDistributionChart.update();
    }

    updateApplicationActivityChart() {
        const appCounts = {};

        this.logMessages.forEach(log => {
            appCounts[log.application] = (appCounts[log.application] || 0) + 1;
        });

        const labels = Object.keys(appCounts);
        const data = Object.values(appCounts);

        this.applicationActivityChart.data.labels = labels;
        this.applicationActivityChart.data.datasets[0].data = data;
        this.applicationActivityChart.update();
    }

    updateSystemHealthChart() {
        const healthScore = this.calculateSystemHealthScore();
        this.systemHealthChart.data.datasets[0].value = healthScore;
        this.systemHealthChart.update();
    }

    calculateSystemHealthScore() {
        // Calculate health score based on current metrics
        const errorRate = this.statistics.total_errors || 0;
        const totalMessages = this.statistics.total_messages || 1;
        const errorPercentage = (errorRate / totalMessages) * 100;

        // Health score: 100 - (error percentage * 2) - queue pressure
        const queuePressure = Math.min((this.statistics.queue_size || 0) / 1000 * 10, 20);
        const healthScore = Math.max(0, 100 - (errorPercentage * 2) - queuePressure);

        return Math.round(healthScore);
    }

    applyFilters() {
        const filteredMessages = this.logMessages.filter(this.messageMatchesFilters.bind(this));
        this.updateLogDisplayWithMessages(filteredMessages);
        this.updateCharts();
    }

    updateLogDisplayWithMessages(messages) {
        const logStream = document.getElementById('logStream');
        logStream.innerHTML = '';

        if (messages.length === 0) {
            logStream.innerHTML = '<div class="stream-placeholder"><p>No messages match current filters</p></div>';
            return;
        }

        messages.forEach(log => {
            const logEntry = document.createElement('div');
            logEntry.className = `log-entry ${log.level.toLowerCase()}`;
            logEntry.innerHTML = `
                <strong>[${log.timestamp}]</strong>
                <span class="level">[${log.level}]</span>
                <span class="component">[${log.component}]</span>
                <span class="message">${this.escapeHtml(log.message)}</span>
            `;
            logStream.appendChild(logEntry);
        });
    }

    async refreshData() {
        const refreshBtn = document.getElementById('refreshBtn');
        const originalText = refreshBtn.innerHTML;
        refreshBtn.innerHTML = '<span class="loading"></span> Refreshing...';
        refreshBtn.disabled = true;

        try {
            await this.loadStatistics();
            await this.loadRecentLogs();
            this.updateApplicationFilter();
            this.updateComponentFilter();
        } catch (error) {
            console.error('Error refreshing data:', error);
        } finally {
            refreshBtn.innerHTML = originalText;
            refreshBtn.disabled = false;
        }
    }

    startAutoRefresh() {
        if (this.autoRefreshInterval) {
            clearInterval(this.autoRefreshInterval);
        }

        this.autoRefreshInterval = setInterval(() => {
            if (this.isAutoRefreshEnabled) {
                this.refreshData();
            }
        }, 30000); // Refresh every 30 seconds
    }

    toggleAutoRefresh() {
        const toggleBtn = document.getElementById('toggleAutoRefresh');
        this.isAutoRefreshEnabled = !this.isAutoRefreshEnabled;

        if (this.isAutoRefreshEnabled) {
            toggleBtn.innerHTML = '⏸️ Pause Auto-Refresh';
            this.startAutoRefresh();
        } else {
            toggleBtn.innerHTML = '▶️ Resume Auto-Refresh';
            if (this.autoRefreshInterval) {
                clearInterval(this.autoRefreshInterval);
                this.autoRefreshInterval = null;
            }
        }
    }

    clearLogDisplay() {
        document.getElementById('logStream').innerHTML = '<div class="stream-placeholder"><p>Log display cleared</p></div>';
    }

    exportLogs() {
        const filteredMessages = this.logMessages.filter(this.messageMatchesFilters.bind(this));
        const dataStr = JSON.stringify(filteredMessages, null, 2);
        const dataBlob = new Blob([dataStr], {type: 'application/json'});

        const url = URL.createObjectURL(dataBlob);
        const link = document.createElement('a');
        link.href = url;
        link.download = `asfmlogger_logs_${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.json`;
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);
        URL.revokeObjectURL(url);
    }

    performSearch() {
        const searchTerm = document.getElementById('searchInput').value.trim();
        if (!searchTerm) return;

        const results = this.logMessages.filter(log =>
            log.message.toLowerCase().includes(searchTerm.toLowerCase()) ||
            log.component.toLowerCase().includes(searchTerm.toLowerCase()) ||
            log.application.toLowerCase().includes(searchTerm.toLowerCase())
        );

        this.displaySearchResults(results, searchTerm);
    }

    displaySearchResults(results, searchTerm) {
        const searchResults = document.getElementById('searchResults');
        searchResults.innerHTML = '';

        if (results.length === 0) {
            searchResults.innerHTML = '<div class="search-placeholder"><p>No results found</p></div>';
            return;
        }

        const header = document.createElement('div');
        header.className = 'search-header';
        header.innerHTML = `<h4>Search Results for "${searchTerm}" (${results.length} found)</h4>`;
        searchResults.appendChild(header);

        results.forEach(log => {
            const resultItem = document.createElement('div');
            resultItem.className = `log-entry ${log.level.toLowerCase()}`;
            resultItem.innerHTML = `
                <strong>[${log.timestamp}]</strong>
                <span class="level">[${log.level}]</span>
                <span class="component">[${log.component}]</span>
                <span class="message">${this.escapeHtml(log.message)}</span>
            `;
            searchResults.appendChild(resultItem);
        });
    }

    showAdvancedSearch() {
        document.getElementById('advancedSearchModal').style.display = 'block';
    }

    hideAdvancedSearch() {
        document.getElementById('advancedSearchModal').style.display = 'none';
    }

    performAdvancedSearch() {
        const searchText = document.getElementById('modalSearchText').value;
        const application = document.getElementById('modalApplication').value;
        const component = document.getElementById('modalComponent').value;
        const level = document.getElementById('modalLevel').value;

        // Apply advanced filters
        let results = this.logMessages;

        if (searchText) {
            results = results.filter(log =>
                log.message.toLowerCase().includes(searchText.toLowerCase())
            );
        }

        if (application) {
            results = results.filter(log => log.application === application);
        }

        if (component) {
            results = results.filter(log => log.component === component);
        }

        if (level) {
            results = results.filter(log => log.level === level);
        }

        this.displaySearchResults(results, 'Advanced Search');
        this.hideAdvancedSearch();
    }

    escapeHtml(text) {
        const div = document.createElement('div');
        div.textContent = text;
        return div.innerHTML;
    }

    clearStream() {
        this.logMessages = [];
        this.updateLogDisplay();
    }

    toggleStream() {
        const pauseBtn = document.getElementById('pauseStreamBtn');
        if (pauseBtn.textContent.includes('Pause')) {
            pauseBtn.innerHTML = '▶️ Resume';
            // Unsubscribe from WebSocket
            if (this.websocket) {
                const unsubscribe = { type: 'unsubscribe' };
                this.websocket.send(JSON.stringify(unsubscribe));
            }
        } else {
            pauseBtn.innerHTML = '⏸️ Pause';
            // Resubscribe to WebSocket
            this.subscribeToLogs();
        }
    }
}

// Initialize dashboard when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    window.dashboard = new ASFMLoggerDashboard();
});

// Export for global access
window.ASFMLoggerDashboard = ASFMLoggerDashboard;