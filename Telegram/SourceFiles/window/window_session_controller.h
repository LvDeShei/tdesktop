/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "base/flags.h"
#include "base/object_ptr.h"
#include "base/weak_ptr.h"
#include "base/timer.h"
#include "boxes/gift_premium_box.h" // GiftPremiumValidator.
#include "chat_helpers/compose/compose_show.h"
#include "data/data_chat_participant_status.h"
#include "dialogs/dialogs_key.h"
#include "ui/layers/layer_widget.h"
#include "settings/settings_type.h"
#include "window/window_adaptive.h"
#include "mtproto/sender.h"

class PhotoData;
class MainWidget;
class MainWindow;

namespace Adaptive {
enum class WindowLayout;
} // namespace Adaptive

namespace Data {
struct StoriesContext;
enum class StorySourcesList : uchar;
} // namespace Data

namespace ChatHelpers {
class TabbedSelector;
class EmojiInteractions;
struct FileChosen;
} // namespace ChatHelpers

namespace Main {
class Session;
} // namespace Main

namespace InlineBots {
class AttachWebView;
enum class PeerType : uint8;
using PeerTypes = base::flags<PeerType>;
} // namespace InlineBots

namespace Calls {
struct StartGroupCallArgs;
} // namespace Calls

namespace Passport {
struct FormRequest;
class FormController;
} // namespace Passport

namespace Ui {
class LayerWidget;
enum class ReportReason;
class ChatStyle;
class ChatTheme;
struct ChatThemeKey;
struct ChatPaintContext;
struct ChatThemeBackground;
struct ChatThemeBackgroundData;
class MessageSendingAnimationController;
} // namespace Ui

namespace Data {
struct CloudTheme;
enum class CloudThemeType;
class Thread;
class Forum;
class ForumTopic;
class WallPaper;
} // namespace Data

namespace HistoryView::Reactions {
class CachedIconFactory;
} // namespace HistoryView::Reactions

namespace Window {

using GifPauseReason = ChatHelpers::PauseReason;
using GifPauseReasons = ChatHelpers::PauseReasons;

class SectionMemento;
class Controller;
class FiltersMenu;

enum class ResolveType {
	Default,
	BotApp,
	BotStart,
	AddToGroup,
	AddToChannel,
	ShareGame,
	Mention,
};

struct PeerThemeOverride {
	PeerData *peer = nullptr;
	std::shared_ptr<Ui::ChatTheme> theme;
	EmojiPtr emoji = nullptr;
};
bool operator==(const PeerThemeOverride &a, const PeerThemeOverride &b);
bool operator!=(const PeerThemeOverride &a, const PeerThemeOverride &b);

class DateClickHandler : public ClickHandler {
public:
	DateClickHandler(Dialogs::Key chat, QDate date);

	void setDate(QDate date);
	void onClick(ClickContext context) const override;

private:
	Dialogs::Key _chat;
	base::weak_ptr<Data::ForumTopic> _weak;
	QDate _date;

};

struct SectionShow {
	enum class Way {
		Forward,
		Backward,
		ClearStack,
	};

	struct OriginMessage {
		FullMsgId id;
	};
	using Origin = std::variant<v::null_t, OriginMessage>;

	SectionShow(
		Way way = Way::Forward,
		anim::type animated = anim::type::normal,
		anim::activation activation = anim::activation::normal)
	: way(way)
	, animated(animated)
	, activation(activation) {
	}
	SectionShow(
		anim::type animated,
		anim::activation activation = anim::activation::normal)
	: animated(animated)
	, activation(activation) {
	}

	[[nodiscard]] SectionShow withWay(Way newWay) const {
		return SectionShow(newWay, animated, activation);
	}
	[[nodiscard]] SectionShow withThirdColumn() const {
		auto copy = *this;
		copy.thirdColumn = true;
		return copy;
	}
	[[nodiscard]] SectionShow withChildColumn() const {
		auto copy = *this;
		copy.childColumn = true;
		return copy;
	}

	Way way = Way::Forward;
	anim::type animated = anim::type::normal;
	anim::activation activation = anim::activation::normal;
	bool thirdColumn = false;
	bool childColumn = false;
	bool reapplyLocalDraft = false;
	Origin origin;

};

class SessionController;

class SessionNavigation : public base::has_weak_ptr {
public:
	explicit SessionNavigation(not_null<Main::Session*> session);
	virtual ~SessionNavigation();

	[[nodiscard]] Main::Session &session() const;

	virtual void showSection(
		std::shared_ptr<SectionMemento> memento,
		const SectionShow &params = SectionShow()) = 0;
	virtual void showBackFromStack(
		const SectionShow &params = SectionShow()) = 0;
	virtual not_null<SessionController*> parentController() = 0;

	struct CommentId {
		MsgId id = 0;
	};
	struct ThreadId {
		MsgId id = 0;
	};
	using RepliesByLinkInfo = std::variant<v::null_t, CommentId, ThreadId>;
	struct PeerByLinkInfo {
		std::variant<QString, ChannelId> usernameOrId;
		QString phone;
		MsgId messageId = ShowAtUnreadMsgId;
		StoryId storyId = 0;
		RepliesByLinkInfo repliesInfo;
		ResolveType resolveType = ResolveType::Default;
		QString startToken;
		ChatAdminRights startAdminRights;
		bool startAutoSubmit = false;
		QString botAppName;
		bool botAppForceConfirmation = false;
		QString attachBotUsername;
		std::optional<QString> attachBotToggleCommand;
		InlineBots::PeerTypes attachBotChooseTypes;
		std::optional<QString> voicechatHash;
		FullMsgId clickFromMessageId;
		QString clickFromAttachBotWebviewUrl;
	};
	void showPeerByLink(const PeerByLinkInfo &info);

	void showRepliesForMessage(
		not_null<History*> history,
		MsgId rootId,
		MsgId commentId = 0,
		const SectionShow &params = SectionShow());
	void showTopic(
		not_null<Data::ForumTopic*> topic,
		MsgId itemId = 0,
		const SectionShow &params = SectionShow());
	void showThread(
		not_null<Data::Thread*> thread,
		MsgId itemId = 0,
		const SectionShow &params = SectionShow());

	void showPeerInfo(
		PeerId peerId,
		const SectionShow &params = SectionShow());
	void showPeerInfo(
		not_null<PeerData*> peer,
		const SectionShow &params = SectionShow());
	void showPeerInfo(
		not_null<Data::Thread*> thread,
		const SectionShow &params = SectionShow());

	virtual void showPeerHistory(
		PeerId peerId,
		const SectionShow &params = SectionShow::Way::ClearStack,
		MsgId msgId = ShowAtUnreadMsgId) = 0;
	void showPeerHistory(
		not_null<PeerData*> peer,
		const SectionShow &params = SectionShow::Way::ClearStack,
		MsgId msgId = ShowAtUnreadMsgId);
	void showPeerHistory(
		not_null<History*> history,
		const SectionShow &params = SectionShow::Way::ClearStack,
		MsgId msgId = ShowAtUnreadMsgId);

	void clearSectionStack(
			const SectionShow &params = SectionShow::Way::ClearStack) {
		showPeerHistory(
			PeerId(0),
			params,
			ShowAtUnreadMsgId);
	}

	void showSettings(
		Settings::Type type,
		const SectionShow &params = SectionShow());
	void showSettings(const SectionShow &params = SectionShow());

	void showPollResults(
		not_null<PollData*> poll,
		FullMsgId contextId,
		const SectionShow &params = SectionShow());

	base::weak_ptr<Ui::Toast::Instance> showToast(
		Ui::Toast::Config &&config);
	base::weak_ptr<Ui::Toast::Instance> showToast(
		TextWithEntities &&text,
		crl::time duration = 0);
	base::weak_ptr<Ui::Toast::Instance> showToast(
		const QString &text,
		crl::time duration = 0);

	[[nodiscard]] virtual std::shared_ptr<ChatHelpers::Show> uiShow();

private:
	void resolvePhone(
		const QString &phone,
		Fn<void(not_null<PeerData*>)> done);
	void resolveUsername(
		const QString &username,
		Fn<void(not_null<PeerData*>)> done);
	void resolveChannelById(
		ChannelId channelId,
		Fn<void(not_null<ChannelData*>)> done);

	void resolveDone(
		const MTPcontacts_ResolvedPeer &result,
		Fn<void(not_null<PeerData*>)> done);

	void showMessageByLinkResolved(
		not_null<HistoryItem*> item,
		const PeerByLinkInfo &info);
	void showPeerByLinkResolved(
		not_null<PeerData*> peer,
		const PeerByLinkInfo &info);
	void joinVoiceChatFromLink(
		not_null<PeerData*> peer,
		const PeerByLinkInfo &info);

	const not_null<Main::Session*> _session;

	MTP::Sender _api;

	mtpRequestId _resolveRequestId = 0;

	History *_showingRepliesHistory = nullptr;
	MsgId _showingRepliesRootId = 0;
	mtpRequestId _showingRepliesRequestId = 0;

};

class SessionController : public SessionNavigation {
public:
	SessionController(
		not_null<Main::Session*> session,
		not_null<Controller*> window);
	~SessionController();

	[[nodiscard]] Controller &window() const {
		return *_window;
	}
	[[nodiscard]] PeerData *singlePeer() const;
	[[nodiscard]] bool isPrimary() const;
	[[nodiscard]] not_null<::MainWindow*> widget() const;
	[[nodiscard]] not_null<MainWidget*> content() const;
	[[nodiscard]] Adaptive &adaptive() const;
	[[nodiscard]] ChatHelpers::EmojiInteractions &emojiInteractions() const {
		return *_emojiInteractions;
	}

	void setConnectingBottomSkip(int skip);
	rpl::producer<int> connectingBottomSkipValue() const;

	using FileChosen = ChatHelpers::FileChosen;
	void stickerOrEmojiChosen(FileChosen chosen);
	[[nodiscard]] rpl::producer<FileChosen> stickerOrEmojiChosen() const;

	QPointer<Ui::BoxContent> show(
		object_ptr<Ui::BoxContent> content,
		Ui::LayerOptions options = Ui::LayerOption::KeepOther,
		anim::type animated = anim::type::normal);
	void hideLayer(anim::type animated = anim::type::normal);

	[[nodiscard]] auto sendingAnimation() const
	-> Ui::MessageSendingAnimationController &;
	[[nodiscard]] auto tabbedSelector() const
	-> not_null<ChatHelpers::TabbedSelector*>;
	void takeTabbedSelectorOwnershipFrom(not_null<QWidget*> parent);
	[[nodiscard]] bool hasTabbedSelectorOwnership() const;

	// This is needed for History TopBar updating when searchInChat
	// is changed in the Dialogs::Widget of the current window.
	rpl::variable<Dialogs::Key> searchInChat;
	bool uniqueChatsInSearchResults() const;

	void openFolder(not_null<Data::Folder*> folder);
	void closeFolder();
	const rpl::variable<Data::Folder*> &openedFolder() const;

	void showForum(
		not_null<Data::Forum*> forum,
		const SectionShow &params = SectionShow::Way::ClearStack);
	void closeForum();
	const rpl::variable<Data::Forum*> &shownForum() const;

	void setActiveChatEntry(Dialogs::RowDescriptor row);
	void setActiveChatEntry(Dialogs::Key key);
	Dialogs::RowDescriptor activeChatEntryCurrent() const;
	Dialogs::Key activeChatCurrent() const;
	rpl::producer<Dialogs::RowDescriptor> activeChatEntryChanges() const;
	rpl::producer<Dialogs::Key> activeChatChanges() const;
	rpl::producer<Dialogs::RowDescriptor> activeChatEntryValue() const;
	rpl::producer<Dialogs::Key> activeChatValue() const;
	bool jumpToChatListEntry(Dialogs::RowDescriptor row);

	void setCurrentDialogsEntryState(Dialogs::EntryState state);
	[[nodiscard]] Dialogs::EntryState currentDialogsEntryState() const;
	bool switchInlineQuery(
		Dialogs::EntryState to,
		not_null<UserData*> bot,
		const QString &query);
	bool switchInlineQuery(
		not_null<Data::Thread*> thread,
		not_null<UserData*> bot,
		const QString &query);

	[[nodiscard]] Dialogs::RowDescriptor resolveChatNext(
		Dialogs::RowDescriptor from = {}) const;
	[[nodiscard]] Dialogs::RowDescriptor resolveChatPrevious(
		Dialogs::RowDescriptor from = {}) const;

	void showEditPeerBox(PeerData *peer);
	void showGiftPremiumBox(UserData *user);

	void enableGifPauseReason(GifPauseReason reason);
	void disableGifPauseReason(GifPauseReason reason);
	rpl::producer<> gifPauseLevelChanged() const {
		return _gifPauseLevelChanged.events();
	}
	bool isGifPausedAtLeastFor(GifPauseReason reason) const;
	void floatPlayerAreaUpdated();

	struct ColumnLayout {
		int bodyWidth = 0;
		int dialogsWidth = 0;
		int chatWidth = 0;
		int thirdWidth = 0;
		Adaptive::WindowLayout windowLayout = Adaptive::WindowLayout();
	};
	[[nodiscard]] ColumnLayout computeColumnLayout() const;
	int dialogsSmallColumnWidth() const;
	bool forceWideDialogs() const;
	void updateColumnLayout();
	bool canShowThirdSection() const;
	bool canShowThirdSectionWithoutResize() const;
	bool takeThirdSectionFromLayer();
	void resizeForThirdSection();
	void closeThirdSection();

	[[nodiscard]] bool canShowSeparateWindow(not_null<PeerData*> peer) const;
	void showPeer(not_null<PeerData*> peer, MsgId msgId = ShowAtUnreadMsgId);

	void startOrJoinGroupCall(not_null<PeerData*> peer);
	void startOrJoinGroupCall(
		not_null<PeerData*> peer,
		Calls::StartGroupCallArgs args);

	void showSection(
		std::shared_ptr<SectionMemento> memento,
		const SectionShow &params = SectionShow()) override;
	void showBackFromStack(
		const SectionShow &params = SectionShow()) override;

	using SessionNavigation::showPeerHistory;
	void showPeerHistory(
		PeerId peerId,
		const SectionShow &params = SectionShow::Way::ClearStack,
		MsgId msgId = ShowAtUnreadMsgId) override;

	void showMessage(
		not_null<const HistoryItem*> item,
		const SectionShow &params = SectionShow::Way::ClearStack);
	void cancelUploadLayer(not_null<HistoryItem*> item);

	void showLayer(
		std::unique_ptr<Ui::LayerWidget> &&layer,
		Ui::LayerOptions options,
		anim::type animated = anim::type::normal);

	void showSpecialLayer(
		object_ptr<Ui::LayerWidget> &&layer,
		anim::type animated = anim::type::normal);
	void hideSpecialLayer(
			anim::type animated = anim::type::normal) {
		showSpecialLayer(nullptr, animated);
	}
	void removeLayerBlackout();
	[[nodiscard]] bool isLayerShown() const;

	void showCalendar(
		Dialogs::Key chat,
		QDate requestedDate);

	void showAddContact();
	void showNewGroup();
	void showNewSupergroup();
	void showNewChannel();

	void showPassportForm(const Passport::FormRequest &request);
	void clearPassportForm();

	struct MessageContext {
		FullMsgId id;
		MsgId topicRootId;
	};
	void openPhoto(
		not_null<PhotoData*> photo,
		MessageContext message,
		const Data::StoriesContext *stories = nullptr);
	void openPhoto(not_null<PhotoData*> photo, not_null<PeerData*> peer);
	void openDocument(
		not_null<DocumentData*> document,
		bool showInMediaView,
		MessageContext message,
		const Data::StoriesContext *stories = nullptr);
	bool openSharedStory(HistoryItem *item);
	bool openFakeItemStory(
		FullMsgId fakeItemId,
		const Data::StoriesContext *stories = nullptr);

	void showChooseReportMessages(
		not_null<PeerData*> peer,
		Ui::ReportReason reason,
		Fn<void(MessageIdsList)> done);
	void clearChooseReportMessages();

	void showInNewWindow(
		not_null<PeerData*> peer,
		MsgId msgId = ShowAtUnreadMsgId);

	void toggleChooseChatTheme(
		not_null<PeerData*> peer,
		std::optional<bool> show = std::nullopt);
	void finishChatThemeEdit(not_null<PeerData*> peer);

	[[nodiscard]] bool dialogsListFocused() const {
		return _dialogsListFocused.current();
	}
	[[nodiscard]] rpl::producer<bool> dialogsListFocusedChanges() const {
		return _dialogsListFocused.changes();
	}
	void setDialogsListFocused(bool value) {
		_dialogsListFocused = value;
	}
	[[nodiscard]] bool dialogsListDisplayForced() const {
		return _dialogsListDisplayForced.current();
	}
	[[nodiscard]] auto dialogsListDisplayForcedChanges() const
	-> rpl::producer<bool> {
		return _dialogsListDisplayForced.changes();
	}
	void setDialogsListDisplayForced(bool value) {
		_dialogsListDisplayForced = value;
	}

	not_null<SessionController*> parentController() override {
		return this;
	}

	[[nodiscard]] int filtersWidth() const;
	[[nodiscard]] rpl::producer<FilterId> activeChatsFilter() const;
	[[nodiscard]] FilterId activeChatsFilterCurrent() const;
	void setActiveChatsFilter(
		FilterId id,
		const SectionShow &params = SectionShow::Way::ClearStack);

	void toggleFiltersMenu(bool enabled);
	[[nodiscard]] rpl::producer<> filtersMenuChanged() const;
	void reloadFiltersMenu();

	[[nodiscard]] auto defaultChatTheme() const
	-> const std::shared_ptr<Ui::ChatTheme> & {
		return _defaultChatTheme;
	}
	[[nodiscard]] auto cachedChatThemeValue(
		const Data::CloudTheme &data,
		const Data::WallPaper &paper,
		Data::CloudThemeType type)
	-> rpl::producer<std::shared_ptr<Ui::ChatTheme>>;
	[[nodiscard]] bool chatThemeAlreadyCached(
		const Data::CloudTheme &data,
		const Data::WallPaper &paper,
		Data::CloudThemeType type);
	void setChatStyleTheme(const std::shared_ptr<Ui::ChatTheme> &theme);
	void clearCachedChatThemes();
	void pushLastUsedChatTheme(const std::shared_ptr<Ui::ChatTheme> &theme);
	[[nodiscard]] not_null<Ui::ChatTheme*> currentChatTheme() const;

	void overridePeerTheme(
		not_null<PeerData*> peer,
		std::shared_ptr<Ui::ChatTheme> theme,
		EmojiPtr emoji);
	void clearPeerThemeOverride(not_null<PeerData*> peer);
	[[nodiscard]] auto peerThemeOverrideValue() const
		-> rpl::producer<PeerThemeOverride> {
		return _peerThemeOverride.value();
	}

	void openPeerStory(
		not_null<PeerData*> peer,
		StoryId storyId,
		Data::StoriesContext context);
	void openPeerStories(
		PeerId peerId,
		std::optional<Data::StorySourcesList> list = std::nullopt);

	struct PaintContextArgs {
		not_null<Ui::ChatTheme*> theme;
		int visibleAreaTop = 0;
		int visibleAreaTopGlobal = 0;
		int visibleAreaWidth = 0;
		QRect clip;
	};
	[[nodiscard]] Ui::ChatPaintContext preparePaintContext(
		PaintContextArgs &&args);
	[[nodiscard]] not_null<const Ui::ChatStyle*> chatStyle() const {
		return _chatStyle.get();
	}

	[[nodiscard]] auto cachedReactionIconFactory() const
	-> HistoryView::Reactions::CachedIconFactory & {
		return *_cachedReactionIconFactory;
	}

	[[nodiscard]] QString authedName() const {
		return _authedName;
	}

	void setPremiumRef(const QString &ref);
	[[nodiscard]] QString premiumRef() const;

	[[nodiscard]] bool contentOverlapped(QWidget *w, QPaintEvent *e);

	[[nodiscard]] std::shared_ptr<ChatHelpers::Show> uiShow() override;

	[[nodiscard]] rpl::lifetime &lifetime() {
		return _lifetime;
	}

private:
	struct CachedThemeKey;
	struct CachedTheme;

	void init();
	void initSupportMode();
	void refreshFiltersMenu();
	void checkOpenedFilter();
	void suggestArchiveAndMute();
	void activateFirstChatsFilter();

	int minimalThreeColumnWidth() const;
	int countDialogsWidthFromRatio(int bodyWidth) const;
	int countThirdColumnWidthFromRatio(int bodyWidth) const;
	struct ShrinkResult {
		int dialogsWidth;
		int thirdWidth;
	};
	ShrinkResult shrinkDialogsAndThirdColumns(
		int dialogsWidth,
		int thirdWidth,
		int bodyWidth) const;

	void pushToChatEntryHistory(Dialogs::RowDescriptor row);
	bool chatEntryHistoryMove(int steps);
	void resetFakeUnreadWhileOpened();

	void checkInvitePeek();
	void setupPremiumToast();

	void pushDefaultChatBackground();
	void cacheChatTheme(
		CachedThemeKey key,
		const Data::CloudTheme &data,
		const Data::WallPaper &paper,
		Data::CloudThemeType type);
	void cacheChatThemeDone(std::shared_ptr<Ui::ChatTheme> result);
	void updateCustomThemeBackground(CachedTheme &theme);
	[[nodiscard]] Ui::ChatThemeBackgroundData backgroundData(
		CachedTheme &theme,
		bool generateGradient = true) const;

	const not_null<Controller*> _window;
	const std::unique_ptr<ChatHelpers::EmojiInteractions> _emojiInteractions;
	const bool _isPrimary = false;

	mutable std::shared_ptr<ChatHelpers::Show> _cachedShow;

	QString _authedName;

	using SendingAnimation = Ui::MessageSendingAnimationController;
	const std::unique_ptr<SendingAnimation> _sendingAnimation;

	std::unique_ptr<Passport::FormController> _passportForm;
	std::unique_ptr<FiltersMenu> _filters;

	GifPauseReasons _gifPauseReasons = 0;
	rpl::event_stream<> _gifPauseLevelChanged;

	// Depends on _gifPause*.
	const std::unique_ptr<ChatHelpers::TabbedSelector> _tabbedSelector;

	rpl::variable<Dialogs::RowDescriptor> _activeChatEntry;
	rpl::lifetime _activeHistoryLifetime;
	rpl::variable<bool> _dialogsListFocused = false;
	rpl::variable<bool> _dialogsListDisplayForced = false;
	std::deque<Dialogs::RowDescriptor> _chatEntryHistory;
	int _chatEntryHistoryPosition = -1;
	bool _filtersActivated = false;

	Dialogs::EntryState _currentDialogsEntryState;

	base::Timer _invitePeekTimer;

	rpl::variable<FilterId> _activeChatsFilter;

	rpl::variable<int> _connectingBottomSkip;

	rpl::event_stream<ChatHelpers::FileChosen> _stickerOrEmojiChosen;

	PeerData *_showEditPeer = nullptr;
	rpl::variable<Data::Folder*> _openedFolder;
	rpl::variable<Data::Forum*> _shownForum;
	rpl::lifetime _shownForumLifetime;

	rpl::event_stream<> _filtersMenuChanged;

	std::shared_ptr<Ui::ChatTheme> _defaultChatTheme;
	base::flat_map<CachedThemeKey, CachedTheme> _customChatThemes;
	rpl::event_stream<std::shared_ptr<Ui::ChatTheme>> _cachedThemesStream;
	const std::unique_ptr<Ui::ChatStyle> _chatStyle;
	std::weak_ptr<Ui::ChatTheme> _chatStyleTheme;
	std::deque<std::shared_ptr<Ui::ChatTheme>> _lastUsedCustomChatThemes;
	rpl::variable<PeerThemeOverride> _peerThemeOverride;

	using ReactionIconFactory = HistoryView::Reactions::CachedIconFactory;
	std::unique_ptr<ReactionIconFactory> _cachedReactionIconFactory;

	base::has_weak_ptr _storyOpenGuard;

	GiftPremiumValidator _giftPremiumValidator;

	QString _premiumRef;

	rpl::lifetime _lifetime;

};

void ActivateWindow(not_null<SessionController*> controller);

[[nodiscard]] bool IsPaused(
	not_null<SessionController*> controller,
	GifPauseReason level);
[[nodiscard]] Fn<bool()> PausedIn(
	not_null<SessionController*> controller,
	GifPauseReason level);

} // namespace Window
