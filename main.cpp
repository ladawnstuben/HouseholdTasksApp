// Group 1 Final Project
// ITCS 2550
// Philip Seros, Wayne Williams, LaDawn Stuben
// 04/28/2024
// Household Task Manager
// This program allows users to create, manage, and track household tasks
// Using wxWidgets for GUI

#include <wx/wx.h> // Needed for wxWidgets
#pragma warning( push )
#pragma warning(disable:26819)
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <fstream>
#include <sstream>
#include <iomanip>
#include <wx/msgdlg.h>  // For wxMessageBox
#include <wx/textdlg.h>  // For wxTextEntryDialog
#include <wx/checkbox.h>  // For wxCheckBox
#include <wx/sizer.h>  // For wxBoxSizer
#include <wx/button.h>  // For wxButton
#include <wx/log.h>
#include <wx/dialog.h>
#pragma warning( pop )

using json = nlohmann::json;
using namespace std;



namespace ChoreAppNamespace {

    const wxString DATA_FILE_PATH = "TestData\\";

    enum class DIFFICULTY { EASY, MEDIUM, HARD };
    enum class STATUS { NOT_STARTED, IN_PROGRESS, COMPLETED };
    enum class PRIORITY { LOW, MODERATE, HIGH };


    //********************************************************************************************************************
    // Client class modified to work with ClientDialog and wxWidgets functions
    class Client {
    private:
        wxString username;
        wxString lastLoggedIn;
        bool notify;
        wxString theme;

    public:
        // Constructor to initialize the client object
        Client(const wxString& username, const wxString& theme, bool notify)
            : username(username), theme(validateTheme(theme)), notify(notify) {
            UpdateLastLoggedIn();
        }
        // Validate theme input
        static wxString validateTheme(const wxString& theme) {
            if (theme == "dark" || theme == "light") {
                return theme;
            }
            else {
                wxLogWarning("Invalid theme specified: %s. Setting default to 'light'.", theme);
                return "light";  // Default theme
            }
        }

        // Method to update the last logged-in time to the current time
        void UpdateLastLoggedIn() {
            time_t now = time(nullptr);  // Get the current time
            struct tm timeinfo;
            localtime_s(&timeinfo, &now);  // Safe conversion from time_t to tm struct
            std::stringstream ss;
            ss << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");
            lastLoggedIn = wxString(ss.str());
        }

        // Setters and getters for username
        void setUsername(const wxString& newUsername) {
            if (!newUsername.IsEmpty()) {
                username = newUsername;
                UpdateLastLoggedIn();  // Update last logged in when username changes
            }
        }

        wxString getUsername() const {
            return username;
        }

        // Setters and getters for notification settings
        void setNotify(bool newNotify) {
            notify = newNotify;
        }

        bool getNotify() const {
            return notify;
        }

        // Setters and getters for theme
        void setTheme(const wxString& newTheme) {
            theme = validateTheme(newTheme);
        }

        wxString getTheme() const {
            return theme;
        }

        // Getter for last logged-in
        wxString getLastLoggedIn() const {
            return lastLoggedIn;
        }

        // Display details - for logging or debugging, not for GUI display
        void printDetails() const {
            wxLogMessage("Username: %s", username);
            wxLogMessage("Last Logged In: %s", lastLoggedIn);
            wxLogMessage("Notification: %s", notify ? "Enabled" : "Disabled");
            wxLogMessage("Theme: %s", theme);
        }
    };

    //*******************************************************************************************************************

    // Define a frame where 'Client' settings can be adjusted
    class ClientDialog : public wxDialog {
    public:
        ClientDialog(wxWindow* parent, Client* client)
            : wxDialog(parent, wxID_ANY, wxT("Edit Client Profile"), wxDefaultPosition, wxSize(350, 200)),
            m_client(client) {

            // Setup UI components
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            m_usernameCtrl = new wxTextCtrl(this, wxID_ANY, m_client->getUsername());
            m_themeCtrl = new wxTextCtrl(this, wxID_ANY, m_client->getTheme());
            m_notifyCtrl = new wxCheckBox(this, wxID_ANY, wxT("Enable Notifications"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
            m_notifyCtrl->SetValue(m_client->getNotify());

            sizer->Add(m_usernameCtrl, 0, wxALL | wxEXPAND, 5);
            sizer->Add(m_themeCtrl, 0, wxALL | wxEXPAND, 5);
            sizer->Add(m_notifyCtrl, 0, wxALL, 5);

            auto* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->AddButton(new wxButton(this, wxID_OK, wxT("Save")));
            buttonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxT("Cancel")));
            buttonSizer->Realize();

            sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);
            SetSizer(sizer);

            // Bind events
            Bind(wxEVT_BUTTON, &ClientDialog::OnSave, this, wxID_OK);
            Bind(wxEVT_BUTTON, &ClientDialog::OnCancel, this, wxID_CANCEL);
        }

    private:
        Client* m_client;
        wxTextCtrl* m_usernameCtrl;
        wxTextCtrl* m_themeCtrl;
        wxCheckBox* m_notifyCtrl;

        void OnSave(wxCommandEvent& event) {
            m_client->setUsername(m_usernameCtrl->GetValue());
            m_client->setTheme(m_themeCtrl->GetValue());
            m_client->setNotify(m_notifyCtrl->GetValue());
            EndModal(wxID_OK);  // Close the dialog with ID_OK
        }

        void OnCancel(wxCommandEvent& event) {
            EndModal(wxID_CANCEL);  // Close the dialog with ID_CANCEL
        }
    };


    //********************************************************************************************************************
    // Chore class modified to work with JSON and wxWidgets functions
    class Chore {
    private:
        int id;
        int earnings;

        wxString name;
        wxString description;
        wxString frequency;
        wxString estimated_time;
        wxString notes;
        wxString location;

        vector<wxString> tags;
        vector<wxString> tools_required;
        vector<wxString> materials_needed;
        vector<wxString> days;
        // Adding callback function for status change
        function<void()> onUpdate;

        DIFFICULTY difficulty;
        STATUS status;
        PRIORITY priority;

    public:
        Chore(const json& j, std::function<void()> callback = nullptr) : onUpdate(callback) {
            id = j["id"].is_null() ? -1 : j["id"].get<int>();
            name = j["name"].is_null() ? wxString("") : wxString(j["name"].get<std::string>());
            description = j["description"].is_null() ? wxString("") : wxString(j["description"].get<std::string>());
            frequency = j["frequency"].is_null() ? wxString("") : wxString(j["frequency"].get<std::string>());
            estimated_time = j["estimated_time"].is_null() ? wxString("") : wxString(j["estimated_time"].get<std::string>());
            earnings = j["earnings"].is_null() ? 0 : j["earnings"].get<int>();

            days = j["days"].is_null() ? vector<wxString>() : parseVectorWXString(j["days"]);
            location = j["location"].is_null() ? wxString("") : wxString(j["location"].get<std::string>());
            tools_required = j["tools_required"].is_null() ? vector<wxString>() : parseVectorWXString(j["tools_required"]);
            materials_needed = j["materials_needed"].is_null() ? vector<wxString>() : parseVectorWXString(j["materials_needed"]);
            notes = j["notes"].is_null() ? wxString("") : wxString(j["notes"].get<std::string>());
            tags = j["tags"].is_null() ? vector<wxString>() : parseVectorWXString(j["tags"]);
            difficulty = parseDifficulty(j);
            priority = parsePriority(j);
            status = parseStatus(j);
        }


        // Default constructor
        Chore() = default;

        virtual ~Chore() {}

        // Helper function to parse a vector of wxString from JSON
        vector<wxString> parseVectorWXString(const json& j) {
            vector<wxString> result;
            if (!j.is_null() && j.is_array()) {
                for (const auto& item : j) {
                    result.push_back(wxString(item.get<std::string>()));
                }
            }
            return result;
        }


        // Call this function to trigger GUI updates
        void triggerUpdate() {
            if (onUpdate) {
                onUpdate();
            }
        }
        // startChore method using wxTextEntryDialog instead of standard input
        void startChore(wxWindow* parent) {
            wxString message;
            if (status == STATUS::NOT_STARTED) {
                message = "In Progress: " + wxString(name);
                status = STATUS::IN_PROGRESS;
            }
            else if (status == STATUS::IN_PROGRESS) {
                message = "Chore already started: " + wxString(name);
            }
            else if (status == STATUS::COMPLETED) {
                message = "In Progress: " + wxString(name);
                status = STATUS::IN_PROGRESS;
            }
            wxMessageBox(message, "Chore Status", wxOK | wxICON_INFORMATION, parent);
            triggerUpdate();  // Trigger any GUI updates if linked
        }

        // completeChore method using wxTextEntryDialog instead of standard input
        void completeChore(wxWindow* parent) {
            wxString message;
            if (status == STATUS::NOT_STARTED) {
                message = "Cannot complete an unstarted chore: " + wxString(name);
            }
            else if (status == STATUS::IN_PROGRESS || status == STATUS::NOT_STARTED) {
                message = "Completed: " + wxString(name);
                status = STATUS::COMPLETED;
            }
            else if (status == STATUS::COMPLETED) {
                message = "Chore already completed: " + wxString(name);
            }
            wxMessageBox(message, "Chore Completion", wxOK | wxICON_INFORMATION, parent);
            triggerUpdate();  // Trigger any GUI updates if linked
        }

        // resetChore method using wxTextEntryDialog instead of standard input
        void resetChore(wxWindow* parent) {
            wxString message;
            if (status == STATUS::NOT_STARTED) {
                message = "Chore not started: " + wxString(name);
            }
            else if (status == STATUS::IN_PROGRESS || status == STATUS::COMPLETED) {
                message = "Resetting: " + wxString(name);
                status = STATUS::NOT_STARTED;
            }
            wxMessageBox(message, "Chore Reset", wxOK | wxICON_INFORMATION, parent);
            triggerUpdate();  // Trigger any GUI updates if linked
        }
        // toJson method to serialize the Chore class object into a JSON format
        json toJSON() const {
            return json{
                {"id", id},
                {"name", name},
                {"description", description},
                {"frequency", frequency},
                {"estimated_time", estimated_time},
                {"earnings", earnings},
                {"days", days},
                {"location", location},
                {"tools_required", tools_required},
                {"materials_needed", materials_needed},
                {"notes", notes},
                {"tags", tags},
                {"difficulty", toStringD(difficulty)},
                {"priority", toStringP(priority)},
                {"status", toStringS(status)}
            };
        }
        // prettyPrint method to display the Chore class object in a readable format
        wxString PrettyPrintClassAttributes() const {
            wxString result = "Chore ID: " + to_string(id) + "\n"
                + "Name: " + name.ToStdString() + "\n"  // Convert wxString to std::string
                + "Description: " + description.ToStdString() + "\n"  // Convert wxString to std::string
                + "Frequency: " + frequency.ToStdString() + "\n"  // Convert wxString to std::string
                + "Estimated Time: " + estimated_time.ToStdString() + "\n"  // Convert wxString to std::string
                + "Earnings: " + std::to_string(earnings) + "\n"
                + "Days: " + formatVector(days) + "\n"  // Ensure formatVector returns std::string
                + "Location: " + location.ToStdString() + "\n"  // Convert wxString to std::string
                + "Tools Required: " + formatVector(tools_required) + "\n"  // Ensure formatVector returns std::string
                + "Materials Needed: " + formatVector(materials_needed) + "\n"  // Ensure formatVector returns std::string
                + "Notes: " + notes.ToStdString() + "\n"  // Convert wxString to std::string
                + "Tags: " + formatVector(tags) + "\n"  // Ensure formatVector returns std::string
                + "Status: " + toStringS(status) + "\n"  // Convert wxString to std::string if needed
                + "Priority: " + toStringP(priority) + "\n"  // Convert wxString to std::string if needed
                + "Difficulty: " + toStringD(difficulty);  // Convert wxString to std::string if needed

            return result;
        }

        // modifyChore method using wxTextEntryDialog
        void modifyChore(wxWindow* parent) {
            // Get the new name of the chore using wxTextEntryDialog
            wxTextEntryDialog nameDialog(parent, wxT("Enter new chore name:"), wxT("Modify Chore Name"), wxString::FromUTF8(name));
            if (nameDialog.ShowModal() == wxID_OK) {
                name = nameDialog.GetValue().ToStdString();
            }

            // Get the new description of the chore using another wxTextEntryDialog
            wxTextEntryDialog descDialog(parent, wxT("Enter new chore description:"), wxT("Modify Chore Description"), wxString::FromUTF8(description));
            if (descDialog.ShowModal() == wxID_OK) {
                description = descDialog.GetValue().ToStdString();
            }

            // Optionally, trigger a GUI update or refresh if necessary
            triggerUpdate();
        }
        // getter and setter methods incorporating triggerUpdate where needed
        int getId() const {
            return id;
        }

        void setId(int newId) {
            id = newId;
            triggerUpdate();
        }

        wxString getName()  {
            return name;
        }

        void setName(const wxString& newName) {
            name = newName;
            triggerUpdate();
        }

        wxString getDescription() const {
            return description;
        }

        void setDescription(const wxString& newDescription) {
            description = newDescription;
            triggerUpdate();
        }

        wxString getFrequency() const {
            return frequency;
        }

        void setFrequency(const wxString& newFrequency) {
            frequency = newFrequency;
            triggerUpdate();
        }

        wxString getEstimatedTime() const {
            return estimated_time;
        }

        void setEstimatedTime(const wxString& newTime) {
            estimated_time = newTime;
            triggerUpdate();
        }

        int getEarnings() const {
            return earnings;
        }

        void setEarnings(int newEarnings) {
            earnings = newEarnings;
            triggerUpdate();
        }

        vector<wxString> getDays() const {
            return days;
        }

        void setDays(const vector<wxString>& newDays) {
            days = newDays;
            triggerUpdate();
        }

        wxString getLocation() const {
            return location;
        }

        void setLocation(const wxString& newLocation) {
            location = newLocation;
            triggerUpdate();
        }

        vector<wxString> getToolsRequired() const {
            return tools_required;
        }

        void setToolsRequired(const vector<wxString>& newTools) {
            tools_required = newTools;
            triggerUpdate();
        }

        vector<wxString> getMaterialsNeeded() const {
            return materials_needed;
        }

        void setMaterialsNeeded(const vector<wxString>& newMaterials) {
            materials_needed = newMaterials;
            triggerUpdate();
        }

        wxString getNotes() const {
            return notes;
        }

        void setNotes(const wxString& newNotes) {
            notes = newNotes;
            triggerUpdate();
        }

        vector<wxString> getTags() const {
            return tags;
        }

        void setTags(const vector<wxString>& newTags) {
            tags = newTags;
            triggerUpdate();
        }

        DIFFICULTY getDifficulty() const {
            return difficulty;
        }

        void setDifficulty(DIFFICULTY newDifficulty) {
            difficulty = newDifficulty;
            triggerUpdate();
        }

        STATUS getStatus() const {
            return status;
        }

        // Friend declaration for operator<<
        friend ostream& operator<<(ostream& os, const Chore& chore);

        virtual bool operator==(const Chore& other) const {
            // Check equality of all member variables
            return (this->id == other.id &&
                this->name == other.name &&
                this->description == other.description &&
                this->frequency == other.frequency &&
                this->estimated_time == other.estimated_time &&
                this->earnings == other.earnings &&
                this->days == other.days &&
                this->location == other.location &&
                this->tools_required == other.tools_required &&
                this->materials_needed == other.materials_needed &&
                this->notes == other.notes &&
                this->tags == other.tags &&
                this->difficulty == other.difficulty &&
                this->status == other.status &&
                this->priority == other.priority);
        }

        struct CompareEarnings {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getEarnings() < b->getEarnings();
            }
        };

        struct CompareDifficulty {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getDifficulty() < b->getDifficulty(); // Make sure Difficulty is comparable
            }
        };

        struct CompareID {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getId() < b->getId();
            }
        };

    protected:
        static wxString formatVector(const vector<wxString>& vec) {
            wxString result;
            for (const auto& item : vec) {
                if (!result.empty()) result += ", ";
                result += wxString::FromUTF8(item.c_str());
            }
            return result.IsEmpty() ? wxString("None") : result;
        }

        // Exception handling in JSON parsing
        DIFFICULTY parseDifficulty(const json& j) {
            try {
                if (j.contains("difficulty") && !j["difficulty"].is_null()) {
                    wxString dif = j["difficulty"].get<wxString>();
                    if (dif == "easy") return DIFFICULTY::EASY;
                    if (dif == "medium") return DIFFICULTY::MEDIUM;
                    if (dif == "hard") return DIFFICULTY::HARD;
                }
            }
            catch (const json::exception& e) {
                // Handle or log the JSON parsing error
                wxLogError("JSON parsing error in parseDifficulty: %s", e.what());
            }
            return DIFFICULTY::EASY;  // Default if parsing fails or is missing
        }

        // parsePriority 
        PRIORITY parsePriority(const json& j) {
            try {
                if (j.contains("priority") && !j["priority"].is_null()) {
                    wxString pri = j["priority"].get<wxString>();
                    if (pri == "low") return PRIORITY::LOW;
                    if (pri == "moderate") return PRIORITY::MODERATE;
                    if (pri == "high") return PRIORITY::HIGH;
                }
            }
            catch (const json::exception& e) {
                // Handle or log the JSON parsing error
                wxLogError("JSON parsing error in parsePriority: %s", e.what());
            }
            return PRIORITY::LOW;  // Default if parsing fails or is missing
        }

        // parseStatus 
        STATUS parseStatus(const json& j) {
            try {
                if (j.contains("status") && !j["status"].is_null()) {
                    wxString stat = j["status"].get<wxString>();
                    if (stat == "not_started") return STATUS::NOT_STARTED;
                    if (stat == "in_progress") return STATUS::IN_PROGRESS;
                    if (stat == "completed") return STATUS::COMPLETED;
                }
            }
            catch (const json::exception& e) {
                // Handle or log the JSON parsing error
                wxLogError("JSON parsing error in parseStatus: %s", e.what());
            }
            return STATUS::NOT_STARTED;  // Default if parsing fails or is missing
        }

        // toStringD using wxString for GUI compatibility
        wxString toStringD(DIFFICULTY d) const {
            switch (d) {
            case DIFFICULTY::EASY: return "easy";
            case DIFFICULTY::MEDIUM: return "medium";
            case DIFFICULTY::HARD: return "hard";
            default: return "easy";
            }
        }


        // toStringP using wxString for GUI compatibility
        wxString toStringP(PRIORITY p) const {
            switch (p) {
            case PRIORITY::LOW: return "low";
            case PRIORITY::MODERATE: return "moderate";
            case PRIORITY::HIGH: return "high";
            default: return "low";
            }
        }


        // toStringS using wxString for GUI compatibilityu
        wxString toStringS(STATUS s) const {
            switch (s) {
            case STATUS::NOT_STARTED: return "not_started";
            case STATUS::IN_PROGRESS: return "in_progress";
            case STATUS::COMPLETED: return "completed";
            default: return "not_started";
            }
        }

    };
    
    // create the ChoreManager class
    class ChoreManager {
    private:
        json j;
        std::vector<std::shared_ptr<Chore>> chores;
        wxString dynamicFile;
        Client* client = nullptr;  // Initialize to nullptr to clearly indicate no client initially


    public:
        ChoreManager(const wxString& fileName) {
            dynamicFile = fileName;
            std::ifstream file(fileName.ToStdString());
            if (file.is_open()) {
                try {
                    j = json::parse(file);
                    file.close();
                }
                catch (const json::parse_error& e) {
                    wxMessageBox("JSON Parsing Error:" + wxString(e.what()), "JSON Error", wxOK | wxICON_ERROR);
                    return;
                }

                // Check for user profile and create Client
                 if (j.contains("user_profile") && !j["user_profile"].is_null()) {
                    auto userProfile = j["user_profile"];
                    wxString username = userProfile.value("username", "defaultUser");
                    wxString theme = userProfile.value("theme", "light");
                    bool notify = userProfile.value("notify", false);

                    client = new Client(username, theme, notify);  // Correctly create a new Client instance
                }
                else {
                    // Default client if no profile is specified
                    client = new Client("defaultUser", "light", false);
                }

                loadChores();
            }
            else {
                wxMessageBox("Error opening file: " + fileName, "File Error", wxOK | wxICON_ERROR);
            }
        }

        ~ChoreManager() {
            saveData();
        }
        void loadChores() {
            if (j.contains("chores") && j["chores"].is_array()) {
                for (const auto& choreJson : j["chores"]) {
                    auto chore = std::make_shared<Chore>(choreJson);
                    chores.push_back(chore);
                }
            }
        }
        void addChore(const json& choreJson) {
            if (!choreJson.is_null()) {
                auto chore = std::make_shared<Chore>(choreJson);
                chores.push_back(chore);
            }
            else {
                wxMessageBox("Error adding chore: Invalid JSON", "JSON Error", wxOK | wxICON_ERROR);

            }
        }
        void saveData() {
            json j;
            if (client) {
                j["user_profile"] = {
                    {"username", client->getUsername().ToStdString()},
                    {"theme", client->getTheme().ToStdString()},
                    {"notify", client->getNotify()}
                };
            }
            json choresJson = json::array();
            for (const auto& chore : chores) {
                choresJson.push_back(chore->toJSON());
            }
            j["chores"] = choresJson;

            std::ofstream file(dynamicFile.ToStdString());
            if (file.is_open()) {
                file << std::setw(4) << j << std::endl;
                file.close();
            }
            else {
                wxMessageBox("Error saving file: " + dynamicFile, "File Error", wxOK | wxICON_ERROR);
            }
        }
        bool validateUser(wxString w,wxString z)
        {
            return true;
        }
        //wxString displayChores() {
        //    wxString info;
        //    for (const auto& chore : chores) {
        //        info += "Chore: " + wxString(chore->getName()) + " (ID: " + wxString::Format(wxT("%d"), chore->getId()) + ")\n";
        //    }
        //    return info;
        //}
        vector<shared_ptr<Chore>> displayChores()
        {
            vector<shared_ptr<Chore>> choreList;
            for (const auto& chore : chores)
            {
                choreList.push_back(chore);
            }
            return choreList;
        }
    };

    // Create the subclass of wxFrame
    // MyFrame serves as the main frame for the application
    //THIS FRAME IS USED WHEN VIEW CHORES IS CLICKED FORM THE CHORES MENU
    //ON THE SECOND WINDOW
    class ChoresFrame : public wxFrame 
    {
    public:
        ChoresFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
            :wxFrame(NULL, wxID_ANY, title, pos, size)
        {
            ChoreManager choreManager(DATA_FILE_PATH + "data.json");
            wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            wxChoice* choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);

           
            choice->Append("Select Chore");

            // Loading chores into the choice menu
            for (const auto& chore : choreManager.displayChores())
            {
                choice->Append(chore->getName());
            }

            sizer->Add(choice, 1, wxEXPAND | wxALL, 5);
            SetSizerAndFit(sizer);

            // Bind the event handler for choice selection
            choice->Bind(wxEVT_CHOICE, &ChoresFrame::OnChoreSelected, this);
        }

    private:
        // Event handler for when a chore is selected from the menu
        void OnChoreSelected(wxCommandEvent& event)
        {
            wxChoice* choice = dynamic_cast<wxChoice*>(event.GetEventObject());
            if (choice)
            {
                wxString selectedChore = choice->GetStringSelection();
                wxMessageBox("Chore Selected: " + selectedChore, "Chore Selection");
            }
        }
    };
    class MyFrame : public wxFrame {
    public:
        MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,int frame1);

    private:
        void CreateMenu(int);
        void CreateControls();
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnButtonClick(wxCommandEvent& event);
        void OnClick(wxCommandEvent& event);
        void OnViewChores(wxCommandEvent& event);
    };

    MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size,int framechoice)
        : wxFrame(nullptr, wxID_ANY, title, pos, size) {
        if (framechoice == 1) {
            CreateMenu(1);
            CreateControls();
        }
    }
    enum
    {
	    ID_CHORE = 1,
        ID_VIEW = 2
    };
    void MyFrame::CreateMenu(int menuChoice) {
        // Creating menu items
        if (menuChoice == 1) {
            //menu items----will be attached to menu names
            wxMenu* menuFile = new wxMenu;
            menuFile->Append(wxID_EXIT, "Exit");

            wxMenu* menuHelp = new wxMenu;
            menuHelp->Append(wxID_ABOUT, "About");
            //making a chore menu that will hopefully display a list of chore
            wxMenu* menuChore = new wxMenu;
            menuChore->Append(ID_CHORE, "&Chores");
            // Menu Items that are shown when clicking the menu name
            wxMenuBar* menuBar = new wxMenuBar;
            menuBar->Append(menuFile, "&File");
            menuBar->Append(menuHelp, "&Help");
            menuBar->Append(menuChore, "&Chores");

            // Setting the menu bar to the frame
            SetMenuBar(menuBar);
        }
        if (menuChoice == 2) {
            //Menu Names
            wxMenu* menuFile = new wxMenu;
            menuFile->Append(wxID_EXIT, "Exit");

            wxMenu* menuHelp = new wxMenu;
            menuHelp->Append(wxID_ABOUT, "About");
            //making a chore menu that will hopefully display a list of chore
            wxMenu* menuChore = new wxMenu;
            menuChore->Append(ID_VIEW, "View Chores");
            // Adding menus to the menu bar
            wxMenuBar* menuBar = new wxMenuBar;
            menuBar->Append(menuFile, "&Manage");
            menuBar->Append(menuHelp, "&SORT");
            menuBar->Append(menuChore, "&VIEW");

            // Setting the menu bar to the frame
            SetMenuBar(menuBar);
            Bind(wxEVT_MENU, &MyFrame::OnViewChores, this, ID_VIEW);
        }

        // Binding events to their handlers
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
        Bind(wxEVT_MENU, &MyFrame::OnClick, this, ID_CHORE);
    }
    //SECOND MENU..... VIEW CHORES
    void MyFrame::OnViewChores(wxCommandEvent& event)
    {
        ChoresFrame* choresFrame = new ChoresFrame("View Chores", wxDefaultPosition, wxDefaultSize);
        choresFrame->Show(true);
    }
    void MyFrame::CreateControls() {
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* vboxSizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* staticText = new wxStaticText(panel, wxID_ANY, wxT("Welcome to Chore Manager!"));
        wxButton* button = new wxButton(panel, wxID_ANY, wxT("Let's Get To Work"));

        vboxSizer->Add(staticText, 1, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);
        vboxSizer->Add(button, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 5);

        panel->SetSizer(vboxSizer);

        // Bind the button event
        button->Bind(wxEVT_BUTTON, &MyFrame::OnButtonClick, this);
    }

    void MyFrame::OnClick(wxCommandEvent& event)
    {
	    if(event.GetId() == 1)
	    {

            wxMessageBox("CLICKED THE CHORES BUTTON");
            CreateMenu(2);
            //Hide();
            
	    }
    }

    void MyFrame::OnExit(wxCommandEvent& event) {
        Close(true);  // Close the frame
    }

    void MyFrame::OnAbout(wxCommandEvent& event) {
        wxMessageBox("This is a wxWidgets application for managing chores.", "About Chore Manager", wxOK | wxICON_INFORMATION);
    }

    // Event handler for the button click
    void MyFrame::OnButtonClick(wxCommandEvent& event) {
        wxMessageBox("Button clicked!", "Event", wxOK | wxICON_INFORMATION);
    }


    // Create the subclass of wxApp
    // ChoreApp serves as the application object
    class ChoreApp : public wxApp {
    public:
        virtual bool OnInit() {
            // Load user profile from the JSON file
            
            ChoreManager choreManager(DATA_FILE_PATH + "data.json");
            LoginDialog* loginFrame = new LoginDialog(nullptr,&choreManager );
            loginFrame->ShowModal();

            // Create and show the main frame
            MyFrame* frame = new MyFrame("Chore Manager", wxPoint(50, 50), wxSize(450, 340),1);
            frame->Show(true);
            return true;
        }
        // This would be a new class for the login dialog
        class LoginDialog : public wxDialog {
        public:
            LoginDialog(wxWindow* parent, ChoreManager* choreManager)
                : wxDialog(parent, wxID_ANY, wxT("Login"), wxDefaultPosition, wxDefaultSize),
                m_choreManager(choreManager), m_isLoggedIn(false) {

                // Setup UI components for login
                auto* sizer = new wxBoxSizer(wxVERTICAL);
                auto* usernameLabel = new wxStaticText(this, wxID_ANY, wxT("Username:"));
                m_usernameText = new wxTextCtrl(this, wxID_ANY, wxT(""));
                auto* passwordLabel = new wxStaticText(this, wxID_ANY, wxT("Password:"));
                m_passwordText = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
                auto* loginButton = new wxButton(this, wxID_OK, wxT("Login"));

                // Add components to sizer
                sizer->Add(usernameLabel, 0, wxALL, 10);
                sizer->Add(m_usernameText, 0, wxEXPAND | wxALL, 10);
                sizer->Add(passwordLabel, 0, wxALL, 10);
                sizer->Add(m_passwordText, 0, wxEXPAND | wxALL, 10);
                sizer->Add(loginButton, 0, wxALIGN_CENTER | wxALL, 10);

                // Set sizer for the dialog
                SetSizer(sizer);

                // Bind login button click event
                loginButton->Bind(wxEVT_BUTTON, &LoginDialog::OnLogin, this);
            }

        private:
            ChoreManager* m_choreManager;
            bool m_isLoggedIn;
            wxTextCtrl* m_usernameText;
            wxTextCtrl* m_passwordText;

            void OnLogin(wxCommandEvent& event) {
                // Get the entered username and password
                wxString username = m_usernameText->GetValue();
                wxString password = m_passwordText->GetValue();

                // Validate the credentials
                if (m_choreManager->validateUser(username, password)) {
                    // Set m_isLoggedIn to true if credentials are valid
                    m_isLoggedIn = true;
                }
                else {
                    // Display an error message if credentials are invalid
                    wxMessageBox("Invalid username or password.", "Login Error", wxOK | wxICON_ERROR);
                }

                // Close the dialog regardless of login success
                EndModal(m_isLoggedIn ? wxID_OK : wxID_CANCEL);
            }
        };
    };

}

wxIMPLEMENT_APP(ChoreAppNamespace::ChoreApp);  // This macro creates the main() function for the application



// Create and show the login dialog
// This isn't working yet
//  LoginDialog* loginDlg = new LoginDialog(nullptr, client);
//   if (loginDlg->ShowModal() == wxID_OK) {
       // User logged in successfully
 //      client->UpdateLastLoggedIn();
       // Save the updated client data back to the JSON file
   //	choreManager.saveData();

//   }
//    else {
        // User cancelled the login
   //     return false; // Exit application
 //   }
 //   loginDlg->Destroy();














































