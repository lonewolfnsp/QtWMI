#include "qtwmi.h"
#include "Wbemidl.h"
#include <Windows.h>
#pragma comment(lib, "Wbemuuid.lib")
#ifdef _DEBUG	
#pragma comment(lib, "comsuppwd.lib")
#else 
#pragma comment(lib, "comsuppw.lib")
#endif

#define SAFE_RELEASE(x){if(x){x->Release(); x=NULL; } }




QtWMI::QtWMI(QWidget *parent, Qt::WFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	connect(ui.btnQuery, SIGNAL(clicked()), this, SLOT(OnQueryWMI()) );
	PopulateWMIClasses();
}

QtWMI::~QtWMI()
{
}

void QtWMI::OnQueryWMI()
{		
	CString strQuery;
	strQuery.Format(L"select * from Win32_%s", ui.cboWMIClasses->currentText().toStdWString().c_str() );	
	IWbemLocator *pLoc = NULL;	
	IWbemServices *pSvc = NULL;
	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject *pclsObj=NULL;
	HRESULT hr=E_FAIL;

	do 
	{
		hr=CoCreateInstance(CLSID_WbemLocator,0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *) &pLoc);
		if (FAILED(hr) )		break;

		hr=pLoc->ConnectServer(	_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
		if (FAILED(hr) )		break;

		hr=CoSetProxyBlanket( pSvc,  RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, 
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE );
		if (FAILED(hr) )		break;

		hr=pSvc->ExecQuery( _bstr_t(L"WQL"),  bstr_t(strQuery), 
			WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,  NULL, &pEnumerator);
		if (FAILED(hr) )		break;

		ULONG		uReturn=0;
		variant_t		vtProp;	
		vtProp.Clear();						
		SAFEARRAY* psaNames=NULL;
		LONG nLower=0, nUpper=0;		
		_bstr_t		PropName;
		CString s;

		while (pEnumerator)
		{
			pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
			if(!uReturn)     break;   

			hr=pclsObj->GetNames(NULL, WBEM_FLAG_ALWAYS | WBEM_FLAG_NONSYSTEM_ONLY, NULL, &psaNames);
			if (hr==WBEM_S_NO_ERROR )
			{
				nLower=nUpper=0;

				SafeArrayGetLBound(psaNames, 1, &nLower);
				SafeArrayGetUBound(psaNames, 1, &nUpper);
				ui.lbDebug->clear();

				for (long i = nLower; i <= nUpper; i++) 
				{					
					SafeArrayGetElement(psaNames, &i, &PropName);	
					s.Format(L"%s", PropName);							
					hr = pclsObj->Get(s, 0, &vtProp, 0, 0);					
					if (hr==WBEM_S_NO_ERROR)																
						DisplayVariantValues(s, vtProp);	
					vtProp.Clear();						
				}					
			}
			if (psaNames)
				::SafeArrayDestroy(psaNames);	
			SAFE_RELEASE(pclsObj);	
		}		

	} while (0);

	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pclsObj);	
	SAFE_RELEASE(pSvc);
	SAFE_RELEASE(pLoc);
}


void QtWMI::PopulateWMIClasses()
{
	ui.cboWMIClasses->clear();
	ui.cboWMIClasses->addItem("BaseBoard");
	ui.cboWMIClasses->addItem("BIOS");
	ui.cboWMIClasses->addItem("BootConfiguration");
	ui.cboWMIClasses->addItem("CDROMDrive");
	ui.cboWMIClasses->addItem("Desktop");
	ui.cboWMIClasses->addItem("DesktopMonitor");
	ui.cboWMIClasses->addItem("DiskDrive");
	ui.cboWMIClasses->addItem("DiskPartition");
	ui.cboWMIClasses->addItem("NetworkAdapter");
	ui.cboWMIClasses->addItem("PhysicalMemory");
	ui.cboWMIClasses->addItem("VideoController");
	ui.cboWMIClasses->addItem("OperatingSystem");	
	ui.cboWMIClasses->addItem("Processor");
	ui.cboWMIClasses->setCurrentIndex (0);
}

void  QtWMI::dprintf(const wchar_t* format, ...)
{	
	wchar_t sz_buf[1024] = {0};	
	va_list argptr;

	va_start(argptr, format);
	 ::wvsprintf(sz_buf, format, argptr);
	va_end(argptr);		
	ui.lbDebug->addItem(QString::fromStdWString(sz_buf) );	
}

void QtWMI::DisplayVariantValues(CString& strName, variant_t& var)
{	
	switch (var.vt)
	{
	case VT_BSTR:
		dprintf(L"%s=%s", strName, var.pbstrVal );		
		break;		

	case VT_UI1:
	case VT_UI2:
	case VT_UI4:
	case VT_UINT:		
		dprintf(L"%s=%lu", strName, var.uiVal);
		break;

	case VT_I1:
	case VT_I2:
	case VT_I4:
	case VT_INT:		
		dprintf(L"%s=%d",strName, var.iVal);
		break;

	case VT_BOOL:		
		dprintf(L"%s=%s",strName, (var.boolVal==VARIANT_TRUE)? L"True": L"False" );
		break;	

	case VT_ARRAY|VT_BSTR:
		{
			SAFEARRAY*	pAray=var.parray;
			LONG  nLower=0, nUpper=0;
			SafeArrayGetLBound(pAray, 1, &nLower);
			SafeArrayGetUBound(pAray, 1, &nUpper);
			_bstr_t		str;										
			for(long n=nLower; n<nUpper;n++)
			{
				SafeArrayGetElement(pAray, &n, &str);				
				dprintf(L"%s=%s",strName , str );
			}							
		}
		break;
	case VT_I4|VT_ARRAY:
		{
			SAFEARRAY*	pAray=var.parray;
			long nLower=0, nUpper=0;
			SafeArrayGetLBound(pAray, 1, &nLower);
			SafeArrayGetUBound(pAray, 1, &nUpper);
			int nVal=0;			
			for(long n=nLower; n<nUpper;n++)
			{
				SafeArrayGetElement(pAray, &n, &nVal);
				dprintf(L"%s=%d",strName, nVal );
			}							
		}
		break;	
	}
}