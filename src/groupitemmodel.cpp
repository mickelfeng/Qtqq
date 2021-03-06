#include "groupitemmodel.h"

#include <QThreadPool>

#include <json/json.h>
#include "core/nameconvertor.h"
#include "core/tasks.h"

void GroupItemModel::parse(const QByteArray &array, NameConvertor *convertor)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(array).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];
    const Json::Value gnamelist = result["gnamelist"];

    for (unsigned int i = 0; i < gnamelist.size(); ++i)
    {
        QString name = QString::fromStdString(gnamelist[i]["name"].asString());
        QString gid = QString::number(gnamelist[i]["gid"].asLargestInt());
        QString code = QString::number(gnamelist[i]["code"].asLargestInt());

        QQItem *item = new QQItem(QQItem::kGroup, name, gid, root_);
        item->set_gCode(code);

        items_.append(item);
        convertor->addUinNameMap(gid, name);;

        root_->children_.append(item);

        if ( item->type() != QQItem::kCategory )
        {
            //GetAvatarTask *task = new GetAvatarTask(item, this);
            //QThreadPool::globalInstance()->start(task);
        }
    }
}
