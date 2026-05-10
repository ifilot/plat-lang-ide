#ifndef BOTTOM_PANEL_H
#define BOTTOM_PANEL_H

#include <QWidget>

class OutputPanel;
class ProblemsPanel;
class QTabWidget;
class TerminalPanel;

/**
 * Hosts the IDE bottom panels: terminal, problems, and output.
 */
class BottomPanel : public QWidget {
    Q_OBJECT

public:
    /**
     * Identifies a bottom panel tab.
     */
    enum class Tab {
        Terminal,
        Problems,
        Output
    };

    /**
     * Creates the bottom panel.
     *
     * @param parent Optional parent widget.
     */
    explicit BottomPanel(QWidget *parent = nullptr);

    /**
     * Returns the terminal panel.
     *
     * @return Terminal panel.
     */
    TerminalPanel *terminal_panel() const;

    /**
     * Appends text to the output panel.
     *
     * @param text Text to append.
     */
    void append_output_message(const QString &text);

    /**
     * Shows or hides one bottom tab.
     *
     * @param tab Tab to update.
     * @param visible Whether the tab should be visible.
     */
    void set_tab_visible(Tab tab, bool visible);

private:
    /**
     * Converts a tab enum to a QTabWidget index.
     *
     * @param tab Tab enum.
     * @return Tab index.
     */
    int tab_index(Tab tab) const;

    /**
     * Keeps the container hidden when all tabs are hidden.
     */
    void update_visibility();

    QTabWidget *tabs_;
    TerminalPanel *terminal_panel_;
    ProblemsPanel *problems_panel_;
    OutputPanel *output_panel_;
};

#endif
