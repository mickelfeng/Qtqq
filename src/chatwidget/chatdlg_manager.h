#ifndef CHATDLG_MANAGER_H
#define CHATDLG_MANAGER_H

#include <QObject>
#include <QVector>

class QQChatDlg;
class MainWindow;
class NameConvertor;
class TabWindow;

class ChatDlgManager : public QObject
{
    Q_OBJECT
signals:
	void activatedChatDlgChanged(QQChatDlg *before, QQChatDlg *after);

public:
	static ChatDlgManager* instance()
	{
		if ( !instance_ )
			instance_ = new ChatDlgManager();
		return instance_;
	}

    ~ChatDlgManager();

	void notifyDlgById(QString id);

    bool isOpening(const QString &id) const;

    QQChatDlg* chatDlgById(QString id) const;

	void setMainWin(MainWindow *main_win);

public slots:
	void openFriendChatDlg(const QString &id);
    void openGroupChatDlg(QString id, QString gcode);
    void openSessChatDlg(QString id, QString group_id);

	void clean();

private slots:
    void closeChatDlg(QQChatDlg *listener);

private:
    QVector<QQChatDlg*> opening_chatdlg_;
    MainWindow *main_win_;
	TabWindow *tab_win_;

private:
	ChatDlgManager();
	ChatDlgManager(const ChatDlgManager &);
	ChatDlgManager &operator=(const ChatDlgManager &);

	static ChatDlgManager *instance_;
};

#endif //CHATDLG_MANAGER_H
