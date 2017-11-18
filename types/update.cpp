#include "update.h"

using namespace Telegram;

Update::Update(QJsonObject update)
{
    id = update.value("update_id").toInt();
    if (update.contains("message"))
        message = Message(update.value("message").toObject());
    else
        if (update.contains("channel_post"))
            message = Message(update.value("channel_post").toObject());

}
