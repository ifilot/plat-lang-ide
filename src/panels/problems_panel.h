#ifndef PROBLEMS_PANEL_H
#define PROBLEMS_PANEL_H

#include <QWidget>

class QLabel;

/**
 * Placeholder surface for diagnostics and compiler problems.
 */
class ProblemsPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates the problems panel.
     *
     * @param parent Optional parent widget.
     */
    explicit ProblemsPanel(QWidget *parent = nullptr);

private:
    QLabel *empty_state_;
};

#endif
