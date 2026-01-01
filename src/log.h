#ifndef LOG_H
#define LOG_H

#include <stdarg.h>


/**
 * @brief Logs a control message
 * @param action Action to be described
 * @param value Description/message associated with the action
 */
void LOG_CTRL(const char *action, const char *value);

/**
 * @brief Logs an error message
 * @param action The action which was being performed when the error occurred
 * @param value Description/message associated with the error (heavily discouraged from multiline)
 * @param msg The actual message (multiline)
 */
void LOG_ERR(const char *action, const char *value, const char *msg);

/**
 * @brief Logs a warning message
 * @param value The warning message
 */
void LOG_WARN(const char *value);

/**
 * @brief Logs an info message
 * @param value The message
 */
void LOG_INFO(const char *value);

/**
 * @brief Logs a printf-formatted info message
 * @param format The message format
 * @param ... Additional arguments
 */
void LOG_INFOF(const char *format, ...);

/**
 * @brief Logs a success message
 * @param value The message
 */
void LOG_YAY(const char *value);

/**
 * @brief Generic logging
 * @param who The 4-char author identifier
 * @param value The message
 */
void LOG(const char *who, const char *value);

/**
 * @brief Confuses dev
 * @param action 
 * @param value Description/message of the fatal error (heavily discouraged from multiline)
 * @param msg The actual message (multiline)
 */
void LOG_FATAL(const char *action, const char *value, const char *msg);

#endif // LOG_H

