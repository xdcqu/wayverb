#include "Main.hpp"
#include "CommandIDs.h"

#include <memory>

const String VisualiserApplication::getApplicationName() {
    return ProjectInfo::projectName;
}
const String VisualiserApplication::getApplicationVersion() {
    return ProjectInfo::versionString;
}
bool VisualiserApplication::moreThanOneInstanceAllowed() {
    return false;
}

void VisualiserApplication::initialise(const String& commandLine) {
    LookAndFeel::setDefaultLookAndFeel(&look_and_feel);

    command_manager = std::make_unique<ApplicationCommandManager>();
    command_manager->registerAllCommandsForTarget(this);

    main_menu_bar_model = std::make_unique<MainMenuBarModel>();

    MenuBarModel::setMacMainMenu(main_menu_bar_model.get(), nullptr);

    //    command_manager->invoke(CommandIDs::idOpenProject, false);
}

void VisualiserApplication::shutdown() {
    main_window = nullptr;

    MenuBarModel::setMacMainMenu(nullptr);

    main_menu_bar_model = nullptr;
    command_manager = nullptr;
}

void VisualiserApplication::systemRequestedQuit() {
    quit();
}

void VisualiserApplication::anotherInstanceStarted(const String& commandLine) {
}

VisualiserApplication::MainWindow::MainWindow(String name, const File& project)
        : DocumentWindow(name, Colours::lightgrey, DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    setContentOwned(new MainContentComponent(project), true);

    centreWithSize(getWidth(), getHeight());
    setVisible(true);

    setResizable(true, false);

    setVisible(true);

    auto& command_manager = VisualiserApplication::get_command_manager();
    command_manager.registerAllCommandsForTarget(this);
    command_manager.getKeyMappings()->resetToDefaultMappings();
    addKeyListener(command_manager.getKeyMappings());
    setWantsKeyboardFocus(false);
}

VisualiserApplication::MainWindow::~MainWindow() noexcept {
    //  TODO if you need to save, do it here (?)

    removeKeyListener(
        VisualiserApplication::get_command_manager().getKeyMappings());
}
void VisualiserApplication::MainWindow::getAllCommands(
    Array<CommandID>& commands) {
    commands.addArray({
        CommandIDs::idSaveAsProject, CommandIDs::idCloseProject,
    });
}
void VisualiserApplication::MainWindow::getCommandInfo(
    CommandID command_id, ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idSaveAsProject:
            result.setInfo("Save As...", "Save as", "General", 0);
            result.defaultKeypresses.add(KeyPress(
                's',
                ModifierKeys::commandModifier | ModifierKeys::shiftModifier,
                0));
            break;
        case CommandIDs::idCloseProject:
            result.setInfo("Close", "Close the current project", "General", 0);
            result.defaultKeypresses.add(
                KeyPress('w', ModifierKeys::commandModifier, 0));
            break;
        default:
            break;
    }
}
bool VisualiserApplication::MainWindow::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idSaveAsProject:
            save_as_project();
            return true;

        case CommandIDs::idCloseProject:
            closeButtonPressed();
            return true;

        default:
            return false;
    }
}

ApplicationCommandTarget*
VisualiserApplication::MainWindow::getNextCommandTarget() {
    return &get_app();
}

void VisualiserApplication::MainWindow::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}

void VisualiserApplication::MainWindow::save_as_project() {
    auto& i = dynamic_cast<MainContentComponent&>(*getContentComponent());
    i.save_as_project();
}

VisualiserApplication& VisualiserApplication::get_app() {
    auto i =
        dynamic_cast<VisualiserApplication*>(JUCEApplication::getInstance());
    jassert(i != nullptr);
    return *i;
}

ApplicationCommandManager& VisualiserApplication::get_command_manager() {
    auto i = VisualiserApplication::get_app().command_manager.get();
    jassert(i);
    return *i;
}

void VisualiserApplication::create_file_menu(PopupMenu& menu) {
    menu.addCommandItem(&get_command_manager(), CommandIDs::idOpenProject);

    menu.addSeparator();

    menu.addCommandItem(&get_command_manager(), CommandIDs::idCloseProject);
    menu.addCommandItem(&get_command_manager(), CommandIDs::idSaveAsProject);

#if !JUCE_MAC
    menu.addSeparator();
    menu.addCommandItem(&get_command_manager(),
                        StandardApplicationCommandIDs::quit);
#endif
}

void VisualiserApplication::handle_main_menu_command(int menu_item_id) {
    if (menu_item_id >= recent_projects_base_id) {
        //  TODO open recent project
    }
}

void VisualiserApplication::getAllCommands(Array<CommandID>& commands) {
    JUCEApplication::getAllCommands(commands);
    commands.addArray({
        CommandIDs::idOpenProject,
    });
}
void VisualiserApplication::getCommandInfo(CommandID command_id,
                                           ApplicationCommandInfo& result) {
    switch (command_id) {
        case CommandIDs::idOpenProject:
            result.setInfo(
                "Open Project...", "Open an existing project", "General", 0);
            result.defaultKeypresses.add(
                KeyPress('o', ModifierKeys::commandModifier, 0));
            break;
        default:
            JUCEApplication::getCommandInfo(command_id, result);
            break;
    }
}

bool VisualiserApplication::perform(const InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::idOpenProject:
            open_project_from_dialog();
            return true;
        default:
            return JUCEApplication::perform(info);
    }
    return true;
}

void VisualiserApplication::open_project(const File& file) {
    main_window = std::make_unique<MainWindow>(getApplicationName(), file);
}

void VisualiserApplication::open_project_from_dialog() {
    FileChooser fc("open project", File::nonexistent, "*.way");
    if (fc.browseForFileToOpen()) {
        open_project(fc.getResult());
    }
}

VisualiserApplication::MainMenuBarModel::MainMenuBarModel() {
    setApplicationCommandManagerToWatch(&get_command_manager());
}

StringArray VisualiserApplication::MainMenuBarModel::getMenuBarNames() {
    return {"File"};
}

PopupMenu VisualiserApplication::MainMenuBarModel::getMenuForIndex(
    int top_level_menu_index, const String& menu_name) {
    PopupMenu menu;
    if (menu_name == "File") {
        get_app().create_file_menu(menu);
    } else {
        jassertfalse;
    }
    return menu;
}

void VisualiserApplication::MainMenuBarModel::menuItemSelected(
    int menu_item_id, int top_level_menu_index) {
    get_app().handle_main_menu_command(menu_item_id);
}

START_JUCE_APPLICATION(VisualiserApplication)
