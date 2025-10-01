/**
 * ASFMLogger MQL5 Wrapper
 *
 * This file provides an MQL5 interface to the ASFMLogger C++ library,
 * enabling MetaTrader Expert Advisors and Indicators to use the enhanced logging capabilities.
 *
 * Usage in MetaTrader:
 * 1. Place this file in your MetaTrader/MQL5/Experts/ or MQL5/Indicators/ directory
 * 2. Include it in your Expert Advisor: #include <ASFMLogger.mq5>
 * 3. Use the logging functions in your EA
 */

//+------------------------------------------------------------------+
//| ASFMLogger MQL5 Wrapper                                         |
//+------------------------------------------------------------------+

// Import functions from ASFMLogger DLL
#import "ASFMLogger.dll"
   void getInstance(string applicationName, string processName);
   void log(void &logger, string level, string component, string function, string message);
   void configureEnhanced(void &logger, string applicationName, bool enableDatabase,
                         string databaseConnection, bool enableSharedMemory, string sharedMemoryName,
                         bool consoleOutput, string logFileName, long maxFileSize,
                         long maxFiles, int logLevel);
#import

// Global logger instance
void globalLogger = NULL;

//+------------------------------------------------------------------+
//| LogMessage class for MQL5                                       |
//+------------------------------------------------------------------+
class CLogMessage
{
private:
   string m_timestamp;
   string m_level;
   string m_component;
   string m_function;
   string m_message;
   string m_symbol;
   int m_magic_number;

public:
   CLogMessage() {}
   ~CLogMessage() {}

   // Constructor
   void CLogMessage::CLogMessage(string level, string message, string component = "MQL5",
                                string function = "", string symbol = "", int magic_number = 0)
     {
      m_timestamp = TimeToString(TimeCurrent(), TIME_DATE|TIME_SECONDS);
      m_level = level;
      m_component = component;
      m_function = function;
      m_message = message;
      m_symbol = symbol;
      m_magic_number = magic_number;
     }

   // Getters
   string GetTimestamp() { return m_timestamp; }
   string GetLevel() { return m_level; }
   string GetComponent() { return m_component; }
   string GetFunction() { return m_function; }
   string GetMessage() { return m_message; }
   string GetSymbol() { return m_symbol; }
   int GetMagicNumber() { return m_magic_number; }

   // Format message for display
   string FormatForDisplay()
     {
      return StringFormat("[%s] [%s] [%s::%s] %s",
                         m_timestamp, m_level, m_component, m_function, m_message);
     }

   // Format message for trading context
   string FormatForTrading()
     {
      if(m_symbol != "" && m_magic_number > 0)
        {
         return StringFormat("[%s] [%s] [%s::%s] Symbol: %s, Magic: %d, Message: %s",
                            m_timestamp, m_level, m_component, m_function,
                            m_symbol, m_magic_number, m_message);
        }
      return FormatForDisplay();
     }
};

//+------------------------------------------------------------------+
//| ASFMLoggerMQL5 class                                             |
//+------------------------------------------------------------------+
class CASFMLoggerMQL5
{
private:
   string m_application_name;
   string m_expert_name;
   string m_symbol;
   int m_magic_number;
   void m_logger;
   bool m_enhanced_enabled;

   // Local message queue for MQL5
   CLogMessage m_local_queue[];
   int m_queue_size;

public:
   CASFMLoggerMQL5() {}
   ~CASFMLoggerMQL5() {}

   // Constructor
   void CASFMLoggerMQL5::CASFMLoggerMQL5(string application_name, string expert_name = "",
                                        string symbol = "", int magic_number = 0)
     {
      m_application_name = application_name;
      m_expert_name = expert_name != "" ? expert_name : application_name;
      m_symbol = symbol != "" ? symbol : _Symbol;
      m_magic_number = magic_number;
      m_queue_size = 0;
      m_enhanced_enabled = false;

      Initialize();
     }

   // Initialize the logger
   void Initialize()
     {
      try
        {
         // Try to initialize C++ logger
         getInstance(m_application_name, m_expert_name);
         m_enhanced_enabled = true;
         Print("Enhanced ASFMLogger initialized for ", m_application_name);
        }
      catch(int error)
        {
         Print("Warning: Enhanced ASFMLogger not available. Error: ", ErrorDescription(error));
         Print("Using local MQL5 logging only.");
         m_enhanced_enabled = false;
        }
     }

   // Enhanced logging methods with trading context
   void Trace(string message, string component = "MQL5", string function = "")
     {
      Log("TRACE", message, component, function);
     }

   void Debug(string message, string component = "MQL5", string function = "")
     {
      Log("DEBUG", message, component, function);
     }

   void Info(string message, string component = "MQL5", string function = "")
     {
      Log("INFO", message, component, function);
     }

   void Warn(string message, string component = "MQL5", string function = "")
     {
      Log("WARN", message, component, function);
     }

   void Error(string message, string component = "MQL5", string function = "")
     {
      Log("ERROR", message, component, function);
     }

   void Critical(string message, string component = "MQL5", string function = "")
     {
      Log("CRITICAL", message, component, function);
     }

   // Core logging method
   void Log(string level, string message, string component, string function)
     {
      try
        {
         // Create formatted message with trading context
         string formatted_message = BuildTradingContextMessage(message);

         // Use C++ enhanced logging if available
         if(m_enhanced_enabled)
           {
            try
              {
               log(m_logger, level, component, function, formatted_message);
              }
            catch(int error)
              {
               Print("Warning: C++ logging failed: ", ErrorDescription(error));
              }
           }

         // Always add to local queue for MQL5 analysis
         AddToLocalQueue(level, message, component, function);

         // Output to MQL5 console
         OutputToMQL5Console(level, formatted_message);

        }
      catch(int error)
        {
         Print("Error in MQL5 logging: ", ErrorDescription(error));
        }
     }

   // Build message with trading context
   string BuildTradingContextMessage(string message)
     {
      string context = "";

      if(m_symbol != "")
        {
         context += "Symbol: " + m_symbol + ", ";
        }

      if(m_magic_number > 0)
        {
         context += "Magic: " + IntegerToString(m_magic_number) + ", ";
        }

      if(AccountInfoString(ACCOUNT_NAME) != "")
        {
         context += "Account: " + AccountInfoString(ACCOUNT_NAME) + ", ";
        }

      context += "Balance: " + DoubleToString(AccountInfoDouble(ACCOUNT_BALANCE), 2) + ", ";
      context += "Equity: " + DoubleToString(AccountInfoDouble(ACCOUNT_EQUITY), 2);

      return message + " | " + context;
     }

   // Add message to local queue
   void AddToLocalQueue(string level, string message, string component, string function)
     {
      if(m_queue_size >= 1000) // Keep only last 1000 messages
        {
         // Shift array to remove oldest message
         for(int i = 0; i < m_queue_size - 1; i++)
           {
            m_local_queue[i] = m_local_queue[i + 1];
           }
         m_queue_size--;
        }

      // Add new message
      CLogMessage new_message(level, message, component, function, m_symbol, m_magic_number);
      m_local_queue[m_queue_size] = new_message;
      m_queue_size++;
     }

   // Output to MQL5 console with color coding
   void OutputToMQL5Console(string level, string message)
     {
      if(level == "TRACE")
        {
         Print("TRACE: ", message);
        }
      else if(level == "DEBUG")
        {
         Print("DEBUG: ", message);
        }
      else if(level == "INFO")
        {
         Print("INFO: ", message);
        }
      else if(level == "WARN")
        {
         Print("WARN: ", message);
        }
      else if(level == "ERROR")
        {
         Print("ERROR: ", message);
         // Also send alert for errors
         Alert("EA Error: ", message);
        }
      else if(level == "CRITICAL")
        {
         Print("CRITICAL: ", message);
         // Send alert for critical errors
         Alert("EA Critical: ", message);
        }
     }

   // Get local log messages
   CLogMessage GetLogMessage(int index)
     {
      if(index >= 0 && index < m_queue_size)
        {
         return m_local_queue[index];
        }
      return CLogMessage();
     }

   // Get recent messages
   string GetRecentMessages(int count = 10)
     {
      string messages = "";
      int start_index = MathMax(0, m_queue_size - count);

      for(int i = start_index; i < m_queue_size; i++)
        {
         CLogMessage msg = m_local_queue[i];
         messages += msg.FormatForTrading() + "\n";
        }

      return messages;
     }

   // Get messages by level
   string GetMessagesByLevel(string level, int count = 50)
     {
      string messages = "";
      int found_count = 0;

      for(int i = m_queue_size - 1; i >= 0 && found_count < count; i--)
        {
         CLogMessage msg = m_local_queue[i];
         if(msg.GetLevel() == level)
           {
            messages += msg.FormatForTrading() + "\n";
            found_count++;
           }
        }

      return messages;
     }

   // Get trading statistics
   string GetTradingStatistics()
     {
      int total_messages = 0;
      int error_count = 0;
      int warning_count = 0;
      int info_count = 0;

      for(int i = 0; i < m_queue_size; i++)
        {
         CLogMessage msg = m_local_queue[i];
         total_messages++;

         if(msg.GetLevel() == "ERROR") error_count++;
         else if(msg.GetLevel() == "WARN") warning_count++;
         else if(msg.GetLevel() == "INFO") info_count++;
        }

      return StringFormat("Trading Statistics:\n" +
                         "Total Messages: %d\n" +
                         "Errors: %d\n" +
                         "Warnings: %d\n" +
                         "Info: %d\n" +
                         "Symbol: %s\n" +
                         "Magic Number: %d",
                         total_messages, error_count, warning_count, info_count,
                         m_symbol, m_magic_number);
     }

   // Export logs to file
   void ExportLogsToFile(string filename)
     {
      int file_handle = FileOpen(filename, FILE_WRITE|FILE_TXT);

      if(file_handle != INVALID_HANDLE)
        {
         for(int i = 0; i < m_queue_size; i++)
           {
            CLogMessage msg = m_local_queue[i];
            string line = msg.FormatForTrading() + "\n";
            FileWriteString(file_handle, line);
           }

         FileClose(file_handle);
         Print("Exported ", m_queue_size, " log messages to ", filename);
        }
      else
        {
         Print("Error: Cannot open file for writing: ", filename);
        }
     }

   // Get application name
   string GetApplicationName() { return m_application_name; }

   // Get symbol
   string GetSymbol() { return m_symbol; }

   // Get magic number
   int GetMagicNumber() { return m_magic_number; }

   // Check if enhanced features are available
   bool IsEnhancedAvailable() { return m_enhanced_enabled; }

   // Get queue size
   int GetQueueSize() { return m_queue_size; }
};

//+------------------------------------------------------------------+
//| Global functions for easy usage                                  |
//+------------------------------------------------------------------+

// Global logger instance
CASFMLoggerMQL5 *GlobalLogger = NULL;

// Initialize global logger
void InitializeGlobalLogger(string application_name, string expert_name = "",
                           string symbol = "", int magic_number = 0)
  {
   if(GlobalLogger == NULL)
     {
      GlobalLogger = new CASFMLoggerMQL5(application_name, expert_name, symbol, magic_number);
     }
  }

// Quick logging functions
void LogTrace(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Trace(message, component, function);
     }
   else
     {
      Print("TRACE: ", message);
     }
  }

void LogDebug(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Debug(message, component, function);
     }
   else
     {
      Print("DEBUG: ", message);
     }
  }

void LogInfo(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Info(message, component, function);
     }
   else
     {
      Print("INFO: ", message);
     }
  }

void LogWarn(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Warn(message, component, function);
     }
   else
     {
      Print("WARN: ", message);
     }
  }

void LogError(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Error(message, component, function);
     }
   else
     {
      Print("ERROR: ", message);
      Alert("EA Error: ", message);
     }
  }

void LogCritical(string message, string component = "MQL5", string function = "")
  {
   if(GlobalLogger != NULL)
     {
      GlobalLogger.Critical(message, component, function);
     }
   else
     {
      Print("CRITICAL: ", message);
      Alert("EA Critical: ", message);
     }
  }

// Trading-specific logging functions
void LogTradeSignal(string signal_type, string symbol, double price, double volume)
  {
   string message = StringFormat("Trade Signal: %s %s at price %.5f, volume %.2f",
                                signal_type, symbol, price, volume);
   LogInfo(message, "Trading", "Signal");
  }

void LogPositionOpen(long ticket, string symbol, int type, double volume, double price)
  {
   string message = StringFormat("Position Opened: Ticket=%d, %s %s, Volume=%.2f, Price=%.5f",
                                ticket, symbol, PositionTypeToString(type), volume, price);
   LogInfo(message, "Trading", "PositionOpen");
  }

void LogPositionClose(long ticket, string symbol, double profit)
  {
   string message = StringFormat("Position Closed: Ticket=%d, %s, Profit=%.2f",
                                ticket, symbol, profit);
   LogInfo(message, "Trading", "PositionClose");
  }

void LogOrderError(int error_code, string operation)
  {
   string message = StringFormat("Order Error: %s failed with code %d (%s)",
                                operation, error_code, ErrorDescription(error_code));
   LogError(message, "Trading", "OrderError");
  }

void LogAccountInfo()
  {
   string message = StringFormat("Account Info: Balance=%.2f, Equity=%.2f, Margin=%.2f, FreeMargin=%.2f",
                                AccountInfoDouble(ACCOUNT_BALANCE),
                                AccountInfoDouble(ACCOUNT_EQUITY),
                                AccountInfoDouble(ACCOUNT_MARGIN),
                                AccountInfoDouble(ACCOUNT_FREEMARGIN));
   LogInfo(message, "Account", "Info");
  }

// Utility function to convert position type to string
string PositionTypeToString(int type)
  {
   if(type == POSITION_TYPE_BUY) return "BUY";
   if(type == POSITION_TYPE_SELL) return "SELL";
   return "UNKNOWN";
  }

//+------------------------------------------------------------------+
//| Expert Advisor integration functions                             |
//+------------------------------------------------------------------+

// Initialize logger in OnInit
void InitializeExpertLogger(string expert_name, string symbol = "", int magic_number = 0)
  {
   InitializeGlobalLogger(expert_name, expert_name, symbol, magic_number);
   LogInfo("Expert Advisor initialized", "System", "OnInit");
   LogAccountInfo();
  }

// Log trading events in OnTick
void LogTickInfo()
  {
   if(GlobalLogger != NULL)
     {
      string message = StringFormat("Tick: %s Ask=%.5f Bid=%.5f Spread=%d",
                                   _Symbol, SymbolInfoDouble(_Symbol, SYMBOL_ASK),
                                   SymbolInfoDouble(_Symbol, SYMBOL_BID),
                                   (int)SymbolInfoInteger(_Symbol, SYMBOL_SPREAD));
      GlobalLogger.Debug(message, "Market", "OnTick");
     }
  }

// Log deinitialization
void LogExpertDeinit(string reason)
  {
   if(GlobalLogger != NULL)
     {
      string message = StringFormat("Expert Advisor deinitialized: %s", reason);
      GlobalLogger.Info(message, "System", "OnDeinit");

      // Export logs if we have any
      if(GlobalLogger.GetQueueSize() > 0)
        {
         string filename = StringFormat("%s_%s_%s.log",
                                       GlobalLogger.GetApplicationName(),
                                       GlobalLogger.GetSymbol(),
                                       IntegerToString(GlobalLogger.GetMagicNumber()));
         GlobalLogger.ExportLogsToFile(filename);
        }
     }
  }

//+------------------------------------------------------------------+
//| Indicator integration functions                                   |
//+------------------------------------------------------------------+

// Initialize logger for custom indicator
void InitializeIndicatorLogger(string indicator_name)
  {
   InitializeGlobalLogger(indicator_name, indicator_name, _Symbol, 0);
   LogInfo("Custom Indicator initialized", "System", "OnInit");
  }

// Log indicator calculations
void LogIndicatorCalculation(string calculation_type, double value, string additional_info = "")
  {
   if(GlobalLogger != NULL)
     {
      string message = StringFormat("Calculation: %s = %.5f", calculation_type, value);
      if(additional_info != "")
        {
         message += ", " + additional_info;
        }
      GlobalLogger.Debug(message, "Indicator", "Calculate");
     }
  }

//+------------------------------------------------------------------+
//| Utility functions for MQL5 integration                           |
//+------------------------------------------------------------------+

// Get current market information as formatted string
string GetMarketInfoString()
  {
   return StringFormat("Market: %s, Time: %s, Ask: %.5f, Bid: %.5f, Spread: %d",
                      _Symbol,
                      TimeToString(TimeCurrent(), TIME_DATE|TIME_SECONDS),
                      SymbolInfoDouble(_Symbol, SYMBOL_ASK),
                      SymbolInfoDouble(_Symbol, SYMBOL_BID),
                      (int)SymbolInfoInteger(_Symbol, SYMBOL_SPREAD));
  }

// Get current account information as formatted string
string GetAccountInfoString()
  {
   return StringFormat("Account: %s, Balance: %.2f, Equity: %.2f, Margin: %.2f",
                      AccountInfoString(ACCOUNT_NAME),
                      AccountInfoDouble(ACCOUNT_BALANCE),
                      AccountInfoDouble(ACCOUNT_EQUITY),
                      AccountInfoDouble(ACCOUNT_MARGIN));
  }

// Log market snapshot
void LogMarketSnapshot()
  {
   if(GlobalLogger != NULL)
     {
      string message = GetMarketInfoString() + " | " + GetAccountInfoString();
      GlobalLogger.Info(message, "Market", "Snapshot");
     }
  }

// Log position information
void LogPositionInfo(long ticket)
  {
   if(PositionSelectByTicket(ticket))
     {
      string message = StringFormat("Position: Ticket=%d, %s %.2f at %.5f, Profit=%.2f",
                                   ticket,
                                   PositionGetString(POSITION_SYMBOL),
                                   PositionGetDouble(POSITION_VOLUME),
                                   PositionGetDouble(POSITION_PRICE_OPEN),
                                   PositionGetDouble(POSITION_PROFIT));
      LogInfo(message, "Position", "Info");
     }
  }

// Log order information
void LogOrderInfo(long ticket)
  {
   if(OrderSelect(ticket))
     {
      string message = StringFormat("Order: Ticket=%d, %s %.2f at %.5f, Type=%s",
                                   ticket,
                                   OrderGetString(ORDER_SYMBOL),
                                   OrderGetDouble(ORDER_VOLUME_CURRENT),
                                   OrderGetDouble(ORDER_PRICE_OPEN),
                                   EnumToString((ENUM_ORDER_TYPE)OrderGetInteger(ORDER_TYPE)));
      LogInfo(message, "Order", "Info");
     }
  }

//+------------------------------------------------------------------+
//| Error handling and logging                                      |
//+------------------------------------------------------------------+

// Log MQL5 runtime errors
void LogMQL5Error(string operation, int error_code)
  {
   string message = StringFormat("MQL5 Error in %s: %d (%s)",
                                operation, error_code, ErrorDescription(error_code));
   LogError(message, "MQL5", operation);
  }

// Log successful operations
void LogSuccessfulOperation(string operation, string details = "")
  {
   string message = StringFormat("Operation successful: %s", operation);
   if(details != "")
     {
      message += " | " + details;
     }
   LogInfo(message, "Operation", operation);
  }

// Log performance metrics
void LogPerformanceMetrics(string operation, long execution_time_ms)
  {
   string message = StringFormat("Performance: %s took %d ms", operation, execution_time_ms);
   LogDebug(message, "Performance", operation);
  }

//+------------------------------------------------------------------+
//| Configuration and setup                                          |
//+------------------------------------------------------------------+

// Configure enhanced logging for MQL5
void ConfigureMQL5Logging(string application_name, bool enable_database = false,
                         string database_connection = "", bool enable_shared_memory = false,
                         string shared_memory_name = "")
  {
   if(GlobalLogger != NULL)
     {
      // Configure enhanced features if C++ library is available
      if(GlobalLogger.IsEnhancedAvailable())
        {
         // Note: This would need to be implemented in the C++ library
         Print("Enhanced logging features available for ", application_name);
        }
      else
        {
         Print("Enhanced logging not available, using local MQL5 logging");
        }
     }
  }

//+------------------------------------------------------------------+
//| Cleanup functions                                                |
//+------------------------------------------------------------------+

// Cleanup logger on deinitialization
void CleanupLogger()
  {
   if(GlobalLogger != NULL)
     {
      // Export final logs
      if(GlobalLogger.GetQueueSize() > 0)
        {
         string filename = StringFormat("logs/%s_final_%s.log",
                                       GlobalLogger.GetApplicationName(),
                                       TimeToString(TimeCurrent(), TIME_DATE));
         StringReplace(filename, ":", "-"); // Remove colons for filename
         GlobalLogger.ExportLogsToFile(filename);
        }

      // Cleanup
      delete GlobalLogger;
      GlobalLogger = NULL;
      Print("ASFMLogger cleanup completed");
     }
  }

//+------------------------------------------------------------------+