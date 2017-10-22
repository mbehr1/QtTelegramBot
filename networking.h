#ifndef NETWORKING_H
#define NETWORKING_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>

#define API_HOST "api.telegram.org"

#define ENDPOINT_GET_ME                     "/getMe"
#define ENDPOINT_SEND_MESSAGE               "/sendMessage"
#define ENDPOINT_FORWARD_MESSAGE            "/forwardMessage"
#define ENDPOINT_SEND_PHOTO                 "/sendPhoto"
#define ENDPOINT_SEND_AUDIO                 "/sendAudio"
#define ENDPOINT_SEND_DOCUMENT              "/sendDocument"
#define ENDPOINT_SEND_STICKER               "/sendSticker"
#define ENDPOINT_SEND_VIDEO                 "/sendVideo"
#define ENDPOINT_SEND_VOICE                 "/sendVoice"
#define ENDPOINT_SEND_LOCATION              "/sendLocation"
#define ENDPOINT_SEND_CHAT_ACTION           "/sendChatAction"
#define ENDPOINT_GET_USER_PROFILE_PHOTOS    "/getUserProfilePhotos"
#define ENDPOINT_GET_UPDATES                "/getUpdates"
#define ENDPOINT_SET_WEBHOOK                "/setWebhook"
#define ENDPOINT_GET_FILE                   "/getFile"

namespace Telegram {

class HttpParameter {
public:
    HttpParameter() = delete; //  : isFile(false) {}
    HttpParameter(const QVariant &aValue, bool aIsFile = false, QString aMimeType = "text/plain", QString aFilename = "") :
        isFile(aIsFile), mimeType(aMimeType), filename(aFilename) {
        if (aValue.canConvert<QByteArray>())
            value = aValue.toByteArray();
        else
            qWarning() << __PRETTY_FUNCTION__ << "can't convert" << aValue;
    }

    QByteArray value;
    bool isFile;
    QString mimeType;
    QString filename;
};

typedef QMap<QString, HttpParameter> ParameterList;

class Networking : public QObject
{
    Q_OBJECT
public:
    Networking(QString token, QObject *parent = 0);
    ~Networking();

    enum Method { GET=1, POST, UPLOAD };

    QByteArray request(const QString &endpoint, const ParameterList &params, Method method);

private:
    QNetworkAccessManager *m_nam;
    QString m_token;

    QUrl buildUrl(QString endpoint);
    QByteArray parameterListToString(const ParameterList &list);

    QByteArray generateMultipartBoundary(const ParameterList &list);
    QByteArray generateMultipartFormData(const ParameterList &list, const QByteArray &boundary);
    QString generateRandomString(int length);
};

}

#endif // NETWORKING_H
