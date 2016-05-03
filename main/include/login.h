#ifndef _LOGIN_H_
#define _LOGIN_H_

enum eLoginError
{
	eLoginOK,
	eLoginBusy,
	eLoginFailed
};

class CAbstractPeer;

class CLogin
{
public:
	CLogin():lastScanRequester(NULL), bWiFiScanInProgress(false){}

	eLoginError startWiFiScan(CAbstractPeer* requester);
	void announceScanCompleted(BssList&);
private:
	CAbstractPeer *lastScanRequester;
	bool bWiFiScanInProgress;
};

extern CLogin Login;

#endif /*_LOGIN_H_*/
