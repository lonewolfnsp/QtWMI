#include "qtwmi.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QtWMI w;
	w.show();
	return a.exec();
}
