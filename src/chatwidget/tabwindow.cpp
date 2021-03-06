#include "tabwindow.h"
#include "ui_tabwindow.h"

#include <assert.h>

#include <QWidget>
#include <QDebug>

#include "chatwidget/qqchatdlg.h"
#include "core/talkable.h"
#include "trayicon/systemtray.h"
#include "skinengine/qqskinengine.h"

TabWindow::TabWindow() :
	ui(new Ui::TabWindow()),
	current_index_(-1)
{
	ui->setupUi(this);
	connect(ui->tab_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
	connect(ui->tab_widget, SIGNAL(currentChanged(int)), this, SLOT(onCurrentTabChanged(int)));

	blink_timer_.setInterval(600);
	connect(&blink_timer_, SIGNAL(timeout()), this, SLOT(onBlinkTimeout()));
}

TabWindow::~TabWindow()
{
	if ( ui )
		delete ui;
	ui = NULL;
}

void TabWindow::activatedTab(QString id)
{
	if ( ((QQChatDlg *)widget(current_index_))->id() == id ) 
		return;

	QQChatDlg *page = pages_.value(id, NULL);

	if ( page )
	{
		ui->tab_widget->setCurrentWidget(page);
	}
}

void TabWindow::addTab(QQChatDlg *page, const QString &label)
{
	Talkable *talkable = page->talkable();
	pages_.insert(talkable->id(), page);
	connect(page, SIGNAL(chatFinish(QQChatDlg * )), this, SLOT(onPageClosting(QQChatDlg *)));
	connect(page->talkable(),  SIGNAL(sigDataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));

	QIcon icon;
	QPixmap pix = talkable->icon();
	if ( !pix.isNull() )
		icon.addPixmap(pix);
	else
	{
		if ( page->type() == QQChatDlg::kFriend )
		{
			icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar")));
		}
		else if ( page->type() == QQChatDlg::kFriend )
		{
			icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));
		}
	}

	int index = ui->tab_widget->addTab(page, icon, label);
	ui->tab_widget->setCurrentIndex(index);
	current_index_ = index;
}

void TabWindow::removeTab(QWidget *page)
{
}

inline
QWidget *TabWindow::widget(int index) const
{
	return ui->tab_widget->widget(index);
}

inline
int TabWindow::indexOf(QWidget *page) const
{
	return ui->tab_widget->indexOf(page);
}

void TabWindow::closeTab(int index)
{
	QWidget *page = widget(index);
	page->close();
}

void TabWindow::onPageClosting(QQChatDlg *page)
{
	int index = indexOf(page);
	if ( index != -1 )
	{
		ui->tab_widget->removeTab(index);
	}
	if ( ui->tab_widget->count() == 0 )		
		hide();
}

void TabWindow::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
	Talkable *talkable = qobject_cast<Talkable *>(sender());
	QQChatDlg *changed_page = pages_.value(talkable->id());
	assert(changed_page);

	switch ( role )
	{
		case TDR_Icon:
			{
				QIcon icon;
				QPixmap pix = data.value<QPixmap>();
				icon.addPixmap(pix);	
				ui->tab_widget->setTabIcon(indexOf(changed_page), icon);
			}
			break;
		case TDR_Status:
			break;
		case TDR_ClientType:
			break;
		default:
			break;
	}
}

void TabWindow::onCurrentTabChanged(int index)
{
	QQChatDlg *before = NULL, *after = NULL;
	if ( current_index_+1 <= pageCount() && current_index_ != -1 &&  index != current_index_ )
	{
		before = (QQChatDlg *)widget(current_index_);
		assert(before);
	}

	if ( index != -1 )
	{
		after = (QQChatDlg *)widget(index);
		QIcon icon;
		QPixmap pix = after->talkable()->icon();
		if ( !pix.isNull() )
			icon.addPixmap(pix);
		setWindowIcon(icon);
		setWindowTitle(after->windowTitle());
		assert(after);
		stopBlink(after);
	}

	current_index_ = index;

	emit activatedPageChanged(before, after);
}

int TabWindow::pageCount() const
{
	return ui->tab_widget->count();
}

void TabWindow::blink(QQChatDlg *dlg)
{
	if ( blinking_dlg_.contains(dlg) )
		return;

	blinking_dlg_.push_back(dlg);
	blink_timer_.start();
}

void TabWindow::onBlinkTimeout()
{
	static QIcon emptyIcon;
	static bool visiable = false;

	if ( emptyIcon.isNull() )
	{
		QPixmap pix(iconSize());
		pix.fill(QColor(0, 0, 0, 0));
		emptyIcon.addPixmap(pix);
	}

	foreach ( QQChatDlg *dlg, blinking_dlg_ )
	{
		int index = indexOf(dlg);
		if ( index != -1 )
		{
			if ( visiable )
			{
				QIcon icon;
				QPixmap pix = dlg->talkable()->icon();
				icon.addPixmap(pix);
				ui->tab_widget->setTabIcon(index, icon);
			}
			else
				ui->tab_widget->setTabIcon(index, emptyIcon);
		}
	}

	visiable = !visiable;
}

void TabWindow::stopBlink(QQChatDlg *dlg)
{
	blinking_dlg_.removeOne(dlg);
	int index = indexOf(dlg);

	QIcon icon;
	QPixmap pix = dlg->talkable()->icon();
	if ( !pix.isNull() )
	{
		icon.addPixmap(pix);
		ui->tab_widget->setTabIcon(index, icon);
	}

	if ( blinking_dlg_.isEmpty() )
	{
		blink_timer_.stop();
	}
}
