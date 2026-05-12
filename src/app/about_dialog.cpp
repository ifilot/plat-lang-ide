#include "about_dialog.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QCoreApplication>
#include <QSysInfo>
#include <QTextBrowser>
#include <QVBoxLayout>

#include "app_version.h"

namespace AboutDialog {

void show(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(QCoreApplication::translate("AboutDialog",
                                                      "About plat-lang IDE"));
    dialog.setModal(true);
    dialog.resize(460, 360);

    auto *title = new QLabel("plat-lang IDE", &dialog);
    QFont title_font = title->font();
    title_font.setPointSize(title_font.pointSize() + 5);
    title_font.setBold(true);
    title->setFont(title_font);

    auto *version = new QLabel(
        QCoreApplication::translate("AboutDialog", "Version %1")
            .arg(app_version()), &dialog);

    auto *details = new QTextBrowser(&dialog);
    details->setOpenExternalLinks(true);
    details->setHtml(QCoreApplication::translate("AboutDialog", R"(
<p>A Qt-based IDE for the <b>platlang</b> language.</p>
<p><b>Qt:</b> %1<br>
<b>Platform:</b> %2</p>
<p><b>Application license:</b> GNU Lesser General Public License v3.</p>
<p>This application uses Qt 6 under the LGPLv3 option. Redistribution must keep
the Qt license notices and preserve the user's LGPL rights for the Qt
libraries.</p>
<p><b>Bundled assets</b></p>
<ul>
  <li>Space Mono font, licensed under the SIL Open Font License 1.1.</li>
  <li>Font Awesome Free icons, licensed under Creative Commons Attribution 4.0.</li>
</ul>
<p>See <code>LICENSE</code> and <code>THIRD_PARTY_NOTICES.md</code> for details.</p>
)").arg(qVersion(), QSysInfo::prettyProductName()));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted,
                     &dialog, &QDialog::accept);

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(10);
    layout->addWidget(title);
    layout->addWidget(version);
    layout->addWidget(details, 1);
    layout->addWidget(buttons);

    dialog.exec();
}

}
