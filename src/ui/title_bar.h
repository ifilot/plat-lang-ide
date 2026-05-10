#ifndef TITLE_BAR_H
#define TITLE_BAR_H

#include <QWidget>

#include "theme_manager.h"

class QColor;
class QLabel;
class QMenuBar;
class QPushButton;

/**
 * Custom frameless-window titlebar with embedded menu and window controls.
 */
class TitleBar : public QWidget {
    Q_OBJECT

public:
    /**
     * Creates a custom titlebar.
     *
     * @param menu_bar Menu bar to embed in the titlebar.
     * @param parent Optional parent widget.
     */
    explicit TitleBar(QMenuBar *menu_bar, QWidget *parent = nullptr);

    /**
     * Updates the displayed window title.
     *
     * @param title Window title.
     */
    void set_title(const QString &title);

    /**
     * Updates control state after the parent window state changes.
     */
    void refresh_window_state();

    /**
     * Applies theme-dependent titlebar assets.
     *
     * @param theme Active theme.
     */
    void apply_theme(ThemeManager::Theme theme);

protected:
    /**
     * Starts native system move behavior when dragging the title area.
     *
     * @param event Mouse press event.
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * Toggles maximized state on titlebar double-click.
     *
     * @param event Mouse double-click event.
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    /**
     * Toggles the top-level window between normal and maximized state.
     */
    void toggle_maximized();

    /**
     * Updates the maximize button text for the current window state.
     */
    void update_window_state();

private:
    /**
     * Returns whether a point is inside a window-control button.
     *
     * @param position Position in titlebar coordinates.
     * @return True when the position hits a control button.
     */
    bool is_control_at(const QPoint &position) const;

    /**
     * Renders the logo as a single-color titlebar mark.
     *
     * @param color Logo color.
     */
    void set_logo_color(const QColor &color);

    QLabel *logo_label_;
    QLabel *title_label_;
    QPushButton *minimize_button_;
    QPushButton *maximize_button_;
    QPushButton *close_button_;
};

#endif
