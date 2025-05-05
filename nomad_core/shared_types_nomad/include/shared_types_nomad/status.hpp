/*
Copyright (c) 2025 Patryk Dudziński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * Authors: Patryk Dudziński
 */

#pragma once
#include "rclcpp/rclcpp.hpp"
#include <string>


/**
 * @file status.hpp
 * @brief  Status class definition.
 */

/**
 * @defgroup Status
 * @brief Return types and function result handling.
 * @{
 */

namespace nomad_core {


class Status;

#define NOMAD_RC_RETURN_ON_ERROR(x)                     \
  do {                                                  \
    Status _x = x;                                      \
    if(!_x.ok()) {                                      \
      return hardware_interface::CallbackReturn::ERROR; \
    }                                                   \
  } while(false)

#define NOMAD_RC_RETURN_ON_ERROR_MSG(x, name, msg)        \
  do {                                                    \
    Status _x = x;                                        \
    if(!_x.ok()) {                                        \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), msg); \
      return hardware_interface::CallbackReturn::ERROR;   \
    }                                                     \
  } while(false)

#define NOMAD_RC_RETURN_ON_ERROR_NAM(x, name)                        \
  do {                                                               \
    Status _x = x;                                                   \
    if(!_x.ok()) {                                                   \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), _x.to_string()); \
      return hardware_interface::CallbackReturn::ERROR;              \
    }                                                                \
  } while(false)

#define NOMAD_RC_ASSING_OR_RETURN(assign, result)       \
  auto _xsar##assign = result;                          \
  do {                                                  \
    if(!_xsar##assign.ok()) {                           \
      return hardware_interface::CallbackReturn::ERROR; \
    }                                                   \
  } while(false);                                       \
  auto assign = std::move(_xsar##assign.valueOrDie());


#define NOMAD_RC_ASSING_TO_OR_RETURN(assign, result)    \
  do {                                                  \
    auto _xsar##assign = result;                        \
    if(!_xsar##assign.ok())                             \
      return hardware_interface::CallbackReturn::ERROR; \
    assign = std::move(_xsar##assign.valueOrDie());     \
  } while(false);


#define NOMAD_RC_ASSIGN_OR_RETURN_MSG(assign, result, name, msg) \
  do {                                                           \
    auto _xsar##assign = result;                                 \
    if(!_xsar##assign.ok()) {                                    \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), msg);        \
      return hardware_interface::CallbackReturn::ERROR;          \
    }                                                            \
    assign = std::move(_xsar##assign.valueOrDie());              \
  } while(false);

#define NOMAD_RC_ASSIGN_OR_RETURN_NAM(assign, result, name)                     \
  do {                                                                          \
    auto _xsar##assign = result;                                                \
    if(!_xsar##assign.ok()) {                                                   \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), _xsar##assign.to_string()); \
      return hardware_interface::CallbackReturn::ERROR;                         \
    }                                                                           \
    assign = std::move(_xsar##assign.valueOrDie());                             \
  } while(false);

#define NOMAD_RC_ASSIGN_TO_OR_RETURN_MSG(assign, result, name, msg) \
  do {                                                              \
    auto _xsar##assign = result;                                    \
    if(!_xsar##assign.ok()) {                                       \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), msg);           \
      return hardware_interface::CallbackReturn::ERROR;             \
    }                                                               \
    assign = std::move(_xsar##assign.valueOrDie());                 \
  } while(false);

#define NOMAD_RC_ASSIGN_TO_OR_RETURN_NAM(assign, result, name)                  \
  do {                                                                          \
    auto _xsar##assign = result;                                                \
    if(!_xsar##assign.ok()) {                                                   \
      RCLCPP_ERROR_STREAM(rclcpp::get_logger(name), _xsar##assign.to_string()); \
      return hardware_interface::CallbackReturn::ERROR;                         \
    }                                                                           \
    assign = std::move(_xsar##assign.valueOrDie());                             \
  } while(false);

/**
 * @brief Macro for returning on error in a single line.
 *
 * This macro is used to return from a function if the result is not OK.
 * usefull to avoid having to write the same check in every function.
 */
#define NOMAD_RETURN_ON_ERROR(x) \
  do {                           \
    Status _x = x.status();      \
    if(!_x.ok())                 \
      return _x;                 \
  } while(false)

/**
 * @brief  Macro for creat new veriable with a value from a result and returning on error in a single line.
 */
#define NOMAD_ASSING_OR_RETURN(assign, result) \
  auto _xsar##assign = result;                 \
  do {                                         \
    if(!_xsar##assign.ok())                    \
      return _xsar##assign.status();           \
  } while(false);                              \
  auto assign = std::move(_xsar##assign.valueOrDie());

/**
 * @brief Macro for assigning a value to a already exisiting veriable from a result and returning on error in a single line.
 *
 */
#define NOMAD_ASSING_TO_OR_RETURN(assign, result)   \
  do {                                              \
    auto _xsar##assign = result;                    \
    if(!_xsar##assign.ok())                         \
      return _xsar##assign.status();                \
    assign = std::move(_xsar##assign.valueOrDie()); \
  } while(false);


/**
 * @enum StatusCode
 * @brief Enumeration representing various statuses. Inspired by ApacheArrow.
 *
 * @var StatusCode::OK
 * Operation was successful.
 *
 * @var StatusCode::OutOfMemory
 * Operation failed due to lack of memory.
 *
 * @var StatusCode::KeyError
 * Operation failed due to a key error.
 *
 * @var StatusCode::TypeError
 * Operation failed due to a type error.
 *
 * @var StatusCode::Invalid
 * Operation failed due to an invalid operation.
 *
 * @var StatusCode::IOError
 * Operation failed due to an I/O error.
 *
 * @var StatusCode::CapacityError
 * Operation failed due to a capacity error.
 *
 * @var StatusCode::IndexError
 * Operation failed due to an index error.
 *
 * @var StatusCode::Cancelled
 * Operation was cancelled.
 *
 * @var StatusCode::UnknownError
 * Operation failed due to an unknown error.
 *
 * @var StatusCode::NotImplemented
 * Operation is not implemented.
 *
 * @var StatusCode::SerializationError
 * Operation failed due to a serialization error.
 *
 * @var StatusCode::RError
 * Operation failed due to an R error.
 *
 * @var StatusCode::CodeGenError
 * Operation failed due to a code generation error.
 *
 * @var StatusCode::ExpressionValidationError
 * Operation failed due to an expression validation error.
 *
 * @var StatusCode::ExecutionError
 * Operation failed due to an execution error.
 *
 * @var StatusCode::AlreadyExists
 * Operation failed because the object already exists.
 *
 * @var StatusCode::TimeOut
 * Operation failed due to a timeout.
 *
 * @var StatusCode::DeviceDisbaled
 * Operation failed because the device is disabled.
 *
 * @var StatusCode::Disconnected
 * Operation failed because the device is disconnected.
 */
enum class StatusCode : char {
  OK                 = 0,
  OutOfMemory        = 1,
  KeyError           = 2,
  TypeError          = 3,
  Invalid            = 4,
  IOError            = 5,
  CapacityError      = 6,
  IndexError         = 7,
  Cancelled          = 8,
  UnknownError       = 9,
  NotImplemented     = 10,
  SerializationError = 11,
  RError             = 13,
  // Gandiva range of errors
  CodeGenError              = 40,
  ExpressionValidationError = 41,
  ExecutionError            = 42,
  // Continue generic codes.
  AlreadyExists = 45,
  TimeOut       = 46,

  DeviceDisbaled = 49,
  Disconnected   = 50
};

/**
 * @brief Status class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status code and a message.
 * For easy use, there are static methods for creating common statuses with some messages.
 * The Status can be converted to a Result object that contains the status and the value.
 */
class Status {
public:
  Status(const Status &status) = default;

  Status(bool value) : _status(StatusCode::OK), _message("") {
    if(!value) {
      _status  = StatusCode::Invalid;
      _message = "Value is false";
    }
  };

  [[nodiscard]] static Status OK() {
    return Status(StatusCode::OK, "");
  };

  [[nodiscard]] static Status OutOfMemory(const std::string msg) {
    return Status(StatusCode::OutOfMemory, msg);
  };

  [[nodiscard]] static Status KeyError(const std::string msg) {
    return Status(StatusCode::KeyError, msg);
  };

  [[nodiscard]] static Status TypeError(const std::string msg) {
    return Status(StatusCode::TypeError, msg);
  };

  [[nodiscard]] static Status Invalid(const std::string msg) {
    return Status(StatusCode::Invalid, msg);
  };

  [[nodiscard]] static Status IOError(const std::string msg) {
    return Status(StatusCode::IOError, msg);
  };

  [[nodiscard]] static Status CapacityError(const std::string msg) {
    return Status(StatusCode::CapacityError, msg);
  };

  [[nodiscard]] static Status IndexError(const std::string msg) {
    return Status(StatusCode::IndexError, msg);
  };

  [[nodiscard]] static Status Cancelled(const std::string msg) {
    return Status(StatusCode::Cancelled, msg);
  };

  [[nodiscard]] static Status UnknownError(const std::string msg) {
    return Status(StatusCode::UnknownError, msg);
  };

  [[nodiscard]] static Status NotImplemented(const std::string msg) {
    return Status(StatusCode::NotImplemented, msg);
  };

  [[nodiscard]] static Status SerializationError(const std::string msg) {
    return Status(StatusCode::SerializationError, msg);
  };

  [[nodiscard]] static Status RError(const std::string msg) {
    return Status(StatusCode::RError, msg);
  };

  [[nodiscard]] static Status CodeGenError(const std::string msg) {
    return Status(StatusCode::CodeGenError, msg);
  };

  [[nodiscard]] static Status ExpressionValidationError(const std::string msg) {
    return Status(StatusCode::ExpressionValidationError, msg);
  };

  [[nodiscard]] static Status ExecutionError(const std::string msg) {
    return Status(StatusCode::ExecutionError, msg);
  };

  [[nodiscard]] static Status AlreadyExists(const std::string msg) {
    return Status(StatusCode::AlreadyExists, msg);
  };

  [[nodiscard]] static Status TimeOut(const std::string msg) {
    return Status(StatusCode::TimeOut, msg);
  };

  [[nodiscard]] static Status DeviceDisbaled(const std::string msg) {
    return Status(StatusCode::DeviceDisbaled, msg);
  };

  [[nodiscard]] static Status Disconnected(const std::string msg) {
    return Status(StatusCode::Disconnected, msg);
  };


  /// @brief get the status
  /// @return 0 if OK or some error code
  [[nodiscard]] StatusCode status_code() {
    return _status;
  };

  [[nodiscard]] Status valueOrDie() {
    return *this;
  };

  /// @brief check if the status is OK
  bool ok() {
    return _status == StatusCode::OK;
  };

  /// @brief get status from status
  [[nodiscard]] Status &status() {
    return *this;
  };

  /// @brief get the message of the status
  [[nodiscard]] const std::string to_string() {
    return _message.empty() ? "OK" : _message;
  };

  bool operator==(const Status &other) const {
    return _status == other._status;
  }

  bool operator!=(const Status &other) const {
    return !(_status != other._status);
  }

private:
  Status(const StatusCode status, std::string message) : _status(status), _message(message){};
  StatusCode _status;
  std::string _message;
};

/**
 * @brief Result class used as return type.
 *
 * This class is used as a return type for functions that can fail. It contains a status and a value.
 * The Result can be converted to a Status object that contains the status and the message.
 *
 */
template <typename T> struct Result {
public:
  /// @brief Create a new Result from status for clean return from functions when error occurs.
  Result(Status status) : _status(status){};

  /// @brief Return a new Result with value and status OK.
  static auto OK(const T &value) -> Result<T> {
    return Result<T>(value, Status::OK());
  }

  static auto Propagate(const T &value, const Status &status) -> Result<T> {
    return Result<T>(value, status);
  }

  /// @brief Get the value of the result or weard error if the status is not OK.
  /// You should check if the status is ok before calling this function.
  [[nodiscard]] T &valueOrDie() {
    return _value;
  }


  /// @brief Get the status of the result.
  [[nodiscard]] Status &status() {
    return _status;
  }

  /// @brief Check if the status is OK.
  [[nodiscard]] bool ok() {
    return _status.ok();
  }

  Result<T> &operator=(const Status &other) {
    return Result<T>(other);
  }

private:
  Result(T value, Status status) : _value(value), _status(status){};
  T _value;
  Status _status;
};

} // namespace nomad_core