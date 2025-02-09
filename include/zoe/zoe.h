/*******************************************************************************
*    Copyright (C) <2019-2024>, winsoft666, <winsoft666@outlook.com>.
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#ifndef ZOE_H_
#define ZOE_H_
#pragma once

#include <string>
#include <memory>
#include <future>
#include <map>

#ifdef ZOE_STATIC
#define ZOE_API
#else
#if defined(ZOE_EXPORTS)
#if defined(_MSC_VER)
#define ZOE_API __declspec(dllexport)
#else
#define ZOE_API
#endif
#else
#if defined(_MSC_VER)
#define ZOE_API __declspec(dllimport)
#else
#define ZOE_API
#endif
#endif
#endif

namespace zoe {
enum class ZoeResult {
  SUCCESSED = 0,
  UNKNOWN_ERROR = 1,
  INVALID_URL = 2,
  INVALID_INDEX_FORMAT = 3,
  INVALID_TARGET_FILE_PATH = 4,
  INVALID_THREAD_NUM = 5,
  INVALID_HASH_POLICY = 6,
  INVALID_SLICE_POLICY = 7,
  INVALID_NETWORK_CONN_TIMEOUT = 8,
  INVALID_NETWORK_READ_TIMEOUT = 9,
  INVALID_FETCH_FILE_INFO_RETRY_TIMES = 10,
  ALREADY_DOWNLOADING = 11,
  CANCELED = 12,
  RENAME_TMP_FILE_FAILED = 13,
  OPEN_INDEX_FILE_FAILED = 14,
  TMP_FILE_EXPIRED = 15,
  INIT_CURL_FAILED = 16,
  INIT_CURL_MULTI_FAILED = 17,
  SET_CURL_OPTION_FAILED = 18,
  ADD_CURL_HANDLE_FAILED = 19,
  CREATE_TARGET_FILE_FAILED = 20,
  CREATE_TMP_FILE_FAILED = 21,
  OPEN_TMP_FILE_FAILED = 22,
  URL_DIFFERENT = 23,
  TMP_FILE_SIZE_ERROR = 24,
  TMP_FILE_CANNOT_RW = 25,
  FLUSH_TMP_FILE_FAILED = 26,
  UPDATE_INDEX_FILE_FAILED = 27,
  SLICE_DOWNLOAD_FAILED = 28,
  HASH_VERIFY_NOT_PASS = 29,
  CALCULATE_HASH_FAILED = 30,
  FETCH_FILE_INFO_FAILED = 31,
  REDIRECT_URL_DIFFERENT = 32,
  NOT_CLEARLY_RESULT = 33,
};

enum class DownloadState {
  Stopped = 0,
  Downloading = 1,
  Paused = 2
};

enum class SlicePolicy {
  Auto = 0,
  FixedSize = 1,
  FixedNum = 2
};

enum class HashType {
  MD5 = 0,
  CRC32 = 1,
  SHA256 = 2
};

enum class HashVerifyPolicy {
  AlwaysVerify = 0,
  OnlyNoFileSize = 1
};

enum class UncompletedSliceSavePolicy {
  AlwaysDiscard = 0,
  SaveExceptFailed = 1
};

class ZOE_API ZoeEvent {
 public:
  ZoeEvent(bool setted = false);
  ~ZoeEvent();

  void set() noexcept;
  void unset() noexcept;
  bool isSetted() noexcept;
  bool wait(int32_t millseconds) noexcept;

 protected:
  ZoeEvent(const ZoeEvent&) = delete;
  ZoeEvent& operator=(const ZoeEvent&) = delete;
  class EventImpl;
  EventImpl* impl_;
};

typedef std::string utf8string;
typedef std::function<void(ZoeResult ret)> ResultFunctor;
typedef std::function<void(int64_t total, int64_t downloaded)> ProgressFunctor;
typedef std::function<void(int64_t byte_per_sec)> RealtimeSpeedFunctor;
typedef std::function<void(const utf8string& verbose)> VerboseOuputFunctor;
typedef std::multimap<utf8string, utf8string> HttpHeaders;

class ZOE_API Zoe {
 public:
  Zoe() noexcept;
  virtual ~Zoe() noexcept;

  static const char* GetResultString(ZoeResult enumVal);

  static void GlobalInit();
  static void GlobalUnInit();

  void setVerboseOutput(VerboseOuputFunctor verbose_functor) noexcept;

  // Pass an int specifying the maximum thread number. zoe will use these threads as much as possible.
  //
  // Set to 0 or negative to switch to the default built-in thread number - 1.
  // The number of threads cannot be greater than 100, otherwise zoe will return INVALID_THREAD_NUM.
  //
  ZoeResult setThreadNum(int32_t thread_num) noexcept;
  int32_t threadNum() const noexcept;

  // Pass an int. It should contain the maximum time in milliseconds that you allow the connection phase to the server to take.
  // This only affects the connection phase, it has no impact once it has connected.
  //
  // Set to 0 or negative to switch to the default built-in connection timeout - 3000 milliseconds.
  //
  ZoeResult setNetworkConnectionTimeout(int32_t milliseconds) noexcept;  // milliseconds
  int32_t networkConnectionTimeout() const noexcept;                     // milliseconds

  // Pass an int specifying the retry times when request file information(such as file size) failed.
  //
  // Set to 0 or negative to switch to the default built-in retry times - 1.
  //
  ZoeResult setRetryTimesOfFetchFileInfo(int32_t retry_times) noexcept;
  int32_t retryTimesOfFetchFileInfo() const noexcept;

  // If use_head is true, zoe will use HEAD method to fetch file info. Otherwise, zoe will use GET method.
  //
  ZoeResult setFetchFileInfoHeadMethodEnabled(bool use_head) noexcept;
  bool fetchFileInfoHeadMethodEnabled() const noexcept;

  // Pass an int as parameter.
  //
  // If the interval seconds that from the saved time of temporary file to present greater than or equal to this parameter,
  //    the temporary file will be discarded.
  //
  // Default to -1, never expired.
  //
  ZoeResult setExpiredTimeOfTmpFile(int32_t seconds) noexcept;  // seconds
  int32_t expiredTimeOfTmpFile() const noexcept;                // seconds

  // Pass an int as parameter.
  //
  // If a download exceeds this speed (counted in bytes per second) the transfer will
  //   pause to keep the speed less than or equal to the parameter value.
  //
  // Defaults is -1, unlimited speed.
  //
  // Set to 0 or negative to switch to the default built-in limit - -1(unlimited speed).
  // This option doesn't affect transfer speeds done with FILE:// URLs.
  //
  ZoeResult setMaxDownloadSpeed(int32_t byte_per_seconds) noexcept;
  int32_t maxDownloadSpeed() const noexcept;

  // Pass an int as parameter.
  //
  // If a download less than this speed (counted in bytes per second) during "low speed time" seconds,
  //   the transfer will be considered as failed.
  //
  // Default is -1, unlimited speed.
  //
  // Set to 0 or negative to switch to the default built-in limit - -1(unlimited speed).
  //
  ZoeResult setMinDownloadSpeed(int32_t byte_per_seconds, int32_t duration) noexcept;  // seconds
  int32_t minDownloadSpeed() const noexcept;
  int32_t minDownloadSpeedDuration() const noexcept;  // seconds

  // Pass an unsigned int specifying your maximal size for the disk cache total buffer in zoe.
  // This buffer size is by default 20971520 byte (20MB).
  //
  ZoeResult setDiskCacheSize(int32_t cache_size) noexcept;  // byte
  int32_t diskCacheSize() const noexcept;                   // byte

  // Set an event, zoe will stop downloading when this event set.
  // If download stopped by stop_event or calling stop, zoe will return CANCELED.
  //
  ZoeResult setStopEvent(ZoeEvent* stop_event) noexcept;
  ZoeEvent* stopEvent() noexcept;

  // Set false, zoe will not check whether the redirected url is the same as in the index file,
  // Default is true, if the redirected url is different from the url in the index file,
  //   zoe will return REDIRECT_URL_DIFFERENT error.
  //
  ZoeResult setRedirectedUrlCheckEnabled(bool enabled) noexcept;
  bool redirectedUrlCheckEnabled() const noexcept;

  // Set true, zoe will parse Content-Md5 header filed and make sure target file's md5 is same as this value,
  //   and in this case, slice files will be expired if content_md5 value that cached in index file is changed.
  // Content-Md5 is pure md5 string, not by base64.
  //
  // Default is false.
  //
  ZoeResult setContentMd5Enabled(bool enabled) noexcept;
  bool contentMd5Enabled() const noexcept;

  // Set slice policy, tell zoe how to calculate each slice size.
  // Default: fixed to 10485760 bytes(10MB)
  //
  ZoeResult setSlicePolicy(SlicePolicy policy, int64_t policy_value) noexcept;
  void slicePolicy(SlicePolicy& policy, int64_t& policy_value) const noexcept;

  // Set hash verify policy, the hash value is the whole file's hash, not for a slice.
  // If fetch file size failed, hash verify is the only way to know whether file download completed.
  // If hash value is empty, will not calculate hash, nor verify hash value.
  //
  ZoeResult setHashVerifyPolicy(HashVerifyPolicy policy,
                                HashType hash_type,
                                const utf8string& hash_value) noexcept;
  void hashVerifyPolicy(HashVerifyPolicy& policy,
                        HashType& hash_type,
                        utf8string& hash_value) const noexcept;

  ZoeResult setHttpHeaders(const HttpHeaders& headers) noexcept;
  HttpHeaders httpHeaders() const noexcept;

  // Set proxy string, such as http://127.0.0.1:8888
  //
  ZoeResult setProxy(const utf8string& proxy) noexcept;
  utf8string proxy() const noexcept;

  // This option determines whether zoe verifies the authenticity of the peer's certificate.
  // This trust is based on a chain of digital signatures, rooted in certification authority (CA) certificates you supply.
  // zoe uses a default bundle of CA certificates (the path for that is determined at build time) and
  //   you can specify alternate certificates with the ca_path option.
  //
  // Default: false and ca_path is empty.
  //
  ZoeResult setVerifyCAEnabled(bool enabled, const utf8string& ca_path) noexcept;
  bool verifyCAEnabled() const noexcept;
  utf8string caPath() const noexcept;

  // This option determines whether zoe verifies that the server cert is for the server it is known as.
  // When negotiating TLS and SSL connections, the server sends a certificate indicating its identity.
  // that certificate must indicate that the server is the server to which you meant to connect, or the connection fails.
  //
  // Default: false
  //
  ZoeResult setVerifyHostEnabled(bool enabled) noexcept;
  bool verifyHostEnabled() const noexcept;

  // Pass a char pointer to a cookie string.
  // See: https://curl.se/libcurl/c/CURLOPT_COOKIELIST.html
  //
  ZoeResult setCookieList(const utf8string& cookie_list) noexcept;
  utf8string cookieList() const noexcept;

  // Set slice save policy when the download task not completed.
  //
  // Default is ALWAYS_DISCARD, because zoe doesn't know how to check slice(especially uncompleted) is valid or not.
  //
  ZoeResult setUncompletedSliceSavePolicy(UncompletedSliceSavePolicy policy) noexcept;
  UncompletedSliceSavePolicy uncompletedSliceSavePolicy() const noexcept;

  // Start to download and change state to DownloadState::Downloading.
  //
  std::shared_future<ZoeResult> start(
      const utf8string& url,
      const utf8string& target_file_path,
      ResultFunctor result_functor,
      ProgressFunctor progress_functor,
      RealtimeSpeedFunctor realtime_speed_functor) noexcept;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  std::shared_future<ZoeResult> start(
      const std::wstring& url,
      const std::wstring& target_file_path,
      ResultFunctor result_functor,
      ProgressFunctor progress_functor,
      RealtimeSpeedFunctor realtime_speed_functor) noexcept;
#endif

  // Pause downloading and change state to DownloadState::Paused.
  //
  void pause() noexcept;

  // Resume downloading and change state to DownloadState::Downloading.
  //
  void resume() noexcept;

  // Stop downloading and change state to DownloadState::Stopped, zoe will return CANCELED in ResultFunctor.
  //
  void stop() noexcept;

  utf8string url() const noexcept;
  utf8string targetFilePath() const noexcept;

  // The file size of server side that will be downloaded.
  // Return -1 when get original file size failed.
  //
  int64_t originFileSize() const noexcept;

  DownloadState state() const noexcept;

  std::shared_future<ZoeResult> futureResult() noexcept;

 protected:
  class ZoeImpl;
  ZoeImpl* impl_;

  Zoe(const Zoe&) = delete;
  Zoe& operator=(const Zoe&) = delete;
};
}  // namespace zoe
#endif  // !ZOE_H_