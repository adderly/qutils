#pragma once
// std
#include <functional>
// Qt
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace zmc
{

namespace Network
{

struct Response {
    Response(QString _data, int _httpCode, QMap<QString, QByteArray> _headers, QNetworkReply::NetworkError error)
        : data(_data)
        , httpCode(_httpCode)
        , networkError(error)
        , headers(_headers)
    {

    }

    QString data;
    int httpCode;
    QNetworkReply::NetworkError networkError;
    QMap<QString, QByteArray> headers;
};

using RequestCallback = std::function<void(const Response &)>;
using UploadProgressCallback = std::function<void(qint64/*bytesSent*/, qint64/*bytesTotal*/)>;

class NetworkManager : public QObject
{
    Q_OBJECT

public:
    enum HttpCodes {
        // Informational
        HTTP_100_CONTINUE = 100,
        HTTP_1010_SWITCHING_PROTOCOLS = 101,
        HTTP_102_PROCESSING = 102,
        // Succcess
        HTTP_200_OK = 200,
        HTTP_201_CREATED = 201,
        HTTP_202_ACCEPTED = 202,
        HTTP_203_NON_AUTH_INFORMATION = 203,
        HTTP_204_NO_CONTENT = 204,
        HTTP_205_RESET_CONTENT = 205,
        HTTP_206_PARTIAL_CONTENT = 206,
        HTTP_207_MULTI_STATUS = 207,
        HTTP_208_ALREADY_REPORTED = 208,
        HTTP_226_IM_USED = 226,
        // Redirection
        HTTP_300_MULTIPLE_CHOICES = 300,
        HTTP_301_MOVED_PERMANENTLY = 301,
        HTTP_302_FOUND = 302,
        HTTP_303_SEE_OTHER = 303,
        HTTP_304_NOT_MODIFIED = 304,
        HTTP_305_USER_PROXY = 305,
        HTTP_307_TEMPORARY_REDIRECT = 307,
        HTTP_308_PERMANENT_REDIRECT = 308,
        // Client Error
        HTTP_400_BAD_REQUEST = 400,
        HTTP_401_UNAUTHORIZED = 401,
        HTTP_402_PAYMENT_REQUIRED = 402,
        HTTP_403_FORBIDDEN = 403,
        HTTP_404_NOT_FOUND = 404,
        HTTP_405_METHOD_NOT_ALLOWED = 405,
        HTTP_406_NOT_ACCEPTABLE = 406,
        HTTP_407_PROXY_AUTHENTICATION_REQUIRED = 407,
        HTTP_408_REQUEST_TIMEOUT = 408,
        HTTP_409_CONFLICT = 409,
        HTTP_410_GONE = 410,
        HTTP_411_LENGTH_REQUIRED = 411,
        HTTP_412_PRECONDITION_FAILED = 412,
        HTTP_413_PAYLOAD_TOO_LARGE = 413,
        HTTP_414_REQUEST_URI_TOO_LONG = 414,
        HTTP_415_UNSUPPORTED_MEDIA_TYPE = 415,
        HTTP_416_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
        HTTP_417_EXPECTATION_FAILED = 417,
        HTTP_418_IM_A_TEAPOT = 418,
        HTTP_421_MISDIRECTED_REQUEST = 421,
        HTTP_422_UNPROCESSABLE_ENTITY = 422,
        HTTP_423_LOCKED = 423,
        HTTP_424_FAILED_DEPENDENCY = 424,
        HTTP_426_UPGRADE_REQUIRED = 426,
        HTTP_428_PRECONDITION_REQUIRED = 428,
        HTTP_429_TOO_MANY_REQUESTS = 429,
        HTTP_421_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
        HTTP_444_CONNECTION_CLOSED_WITHOUT_RESPONSE = 444,
        HTTP_451_UNAVAILABLE_FOR_LEGAL_REASONS = 451,
        HTTP_499_CLIENT_CLOSED_REQUEST = 499,
        // Server Errors
        HTTP_500_INTERNAL_SERVER_ERROR = 500,
        HTTP_501_NOT_IMPLEMENTED = 501,
        HTTP_502_BAD_GATEWAY = 502,
        HTTP_503_SERVICE_UNAVAILABLE = 503,
        HTTP_504_GATEWAY_TIMEOUT = 504,
        HTTP_505_HTTP_VERSION_NOT_SUPPORTED = 505,
        HTTP_506_VARIANT_ALSO_NEGOTIATES = 506,
        HTTP_507_INSUFFICIENT_STORAGE = 507,
        HTTP_508_LOOP_DETECTED = 508,
        HTTP_510_NOT_EXTENDED = 510,
        HTTP_511_NETWORK_AUTHENTICATION_REQUIRED = 511,
        HTTP_599_NETWORK_CONNECT_TIMEOUT_ERROR = 599
    };
    Q_ENUM(HttpCodes);

public:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    /*!
     * \brief Sends a get request. When the request is finished, the callback is called. If the queryParams parameter is provided, the query parameters are
     * appended to the end of the URL.
     * \param url
     * \param queryParams
     * \param callback
     */
    void sendGet(const QString &url, RequestCallback callback, const QVariantMap &queryParams = QVariantMap());

    /*!
     * \brief Sends a head request. When the request is finished, the callback is called. If the queryParams parameter is provided, the query parameters are
     * appended to the end of the URL.
     * \param url
     * \param queryParams
     * \param callback
     */
    void sendHead(const QString &url, RequestCallback callback, const QVariantMap &queryParams = QVariantMap());

    /*!
     * \brief Sends a delete request. When the request is finished, the callback is called.
     * \param url
     * \param callback
     */
    void sendDelete(const QString &url, RequestCallback callback);

    /*!
     * \brief Sends a post request. When the request is finished, the callback is called.
     * \param url
     * \param data
     * \param callback
     */
    void sendPost(const QString &url, const QString &data, RequestCallback callback);

    /*!
     * \brief Sends a put request. When the request is finished, the callback is called.
     * \param url
     * \param data
     * \param callback
     */
    void sendPut(const QString &url, const QString &data, RequestCallback callback);

    /*!
     * \brief Send a put request using a QIODevice.
     * \param url
     * \param data
     * \param callback
     * \return QNetworkReply *
     *
     * You are responsible to keep the \a data open while the request is in progress.
     * The request also sets the Content-Length header to the size of \a data.
     */
    QNetworkReply *sendPut(const QString &url, QIODevice *data, RequestCallback callback);

    /*!
     * \brief Uploads the given files with HTTP multipart.
     * \param url
     * \param files QMap<UPLOAD_KEY, UPLOAD_FILE>
     * \param textParams QMap<UPLOAD_KEY, UPLOAD_VALUE>
     * \param callback
     */
    void sendMultipartRequest(
        const QString &url,
        const QMap<QString, QString> &files,
        const QMap<QString, QString> &textParams,
        RequestCallback callback,
        UploadProgressCallback uploadProgressCallback = nullptr,
        bool usePutRequest = false
    );
    void sendMultipartPost(
        const QString &url,
        const QMap<QString, QString> &files,
        const QMap<QString, QString> &textParams,
        RequestCallback callback,
        UploadProgressCallback uploadProgressCallback = nullptr
    );
    void sendMultipartPut(
        const QString &url,
        const QMap<QString, QString> &files,
        const QMap<QString, QString> &textParams,
        RequestCallback callback,
        UploadProgressCallback uploadProgressCallback = nullptr
    );

    /*!
     * \brief Uses the sendMultipartRequest to upload the given files.
     * \param url
     * \param files
     * \param textParams
     * \param callback
     */
    void uploadFiles(
        const QString &url,
        const QMap<QString, QString> &files,
        const QMap<QString, QString> &textParams,
        RequestCallback callback,
        UploadProgressCallback uploadProgressCallback = nullptr
    );

    /*!
     * \brief Returns true if connected to internet.
     * \todo Fix this function. Currently it always returns true.
     * \return bool
     */
    bool isConnectedToInternet();

    /*!
     * \brief Increases m_RequestCount and returns the resulting ID
     * \return
     */
    int getNextrequestID();

    /*!
     * \brief When a header is set, it is used for all of the requests. If a header with the same headerName exists, it is overwritten.
     * \param headerName
     * \param headerValue
     */
    void setHeader(const QString &headerName, const QString &headerValue);

    /*!
     * \brief Removes the header with the given name.
     * \param headerName
     */
    void removeHeader(const QString &headerName);

private:
    static int m_RequestCount;

    QNetworkAccessManager m_Network;
    QList<RequestCallback> m_Callbacks;
    QMap<QByteArray, QByteArray> m_Headers;

signals:
    void uploadProgressChanged(qint64 bytesSent, qint64 bytesTotal, float percent);

private:
    void onRequestFinished(QNetworkReply *reply);
    QMap<QString, QByteArray> getResponseHeaders(const QNetworkReply *reply);
    void onReceivedResponse(const Response &response, int threadIndex);
    void onUploadProgressChanged(qint64 bytesSent, qint64 bytesTotal);

    /*!
     * \brief Returns the first nullptr thread index. If none found, returns -1
     * \return
     */
    int getAvailableIndex();

    void insertCallback(const int &threadIndex, RequestCallback &&callback);

    /*!
     * \brief If a token exists, sets the Authorization header of the HTTPRequest
     * \param request
     */
    void setHeaders(QNetworkRequest &request);
};

}

}
