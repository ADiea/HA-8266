#ifndef UI_MANAGER
#define UI_MANAGER

enum eUIPage
{
	UI_LoadingPage,
};

class UIManager
{
public:
	UIManager(){}
	void showPage(eUIPage page);

};

extern UIManager UI;

#endif //UI_MANAGER
