#ifndef QTWMI_H
#define QTWMI_H

#include <QtGui/QDialog>
#include "ui_qtwmi.h"
#include <comutil.h>
#include <atlstr.h>		//for using CString 

class QtWMI : public QDialog
{
	Q_OBJECT
	
	Ui::QtWMIClass ui;	
	void PopulateWMIClasses();
	void dprintf(const wchar_t* format, ...);	
	void DisplayVariantValues(CString& strName, variant_t& var);
private slots:
		void OnQueryWMI();

public:
	QtWMI(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QtWMI();
	
};

#endif // QTWMI_H
