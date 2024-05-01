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

            // Add components to the sizer
            sizer->Add(m_usernameCtrl, 0, wxALL | wxEXPAND, 5);
            sizer->Add(m_themeCtrl, 0, wxALL | wxEXPAND, 5);
            sizer->Add(m_notifyCtrl, 0, wxALL, 5);

            // Add OK and Cancel buttons
            auto* buttonSizer = new wxStdDialogButtonSizer();
            buttonSizer->AddButton(new wxButton(this, wxID_OK, wxT("Save")));
            buttonSizer->AddButton(new wxButton(this, wxID_CANCEL, wxT("Cancel")));
            buttonSizer->Realize();

            // Add the button sizer to the main sizer
            sizer->Add(buttonSizer, 0, wxALL | wxALIGN_CENTER, 5);
            SetSizer(sizer);

            // Bind events
            Bind(wxEVT_BUTTON, &ClientDialog::OnSave, this, wxID_OK);
            Bind(wxEVT_BUTTON, &ClientDialog::OnCancel, this, wxID_CANCEL);
        }

    private:
        Client* m_client;          // Pointer to the client object
        wxTextCtrl* m_usernameCtrl;  // Text control for username
        wxTextCtrl* m_themeCtrl;     // Theme 
        wxCheckBox* m_notifyCtrl;    // Checkbox for notifications

        // Event handlers
        void OnSave(wxCommandEvent& event) {
            m_client->setUsername(m_usernameCtrl->GetValue());
            m_client->setTheme(m_themeCtrl->GetValue());
            m_client->setNotify(m_notifyCtrl->GetValue());
            EndModal(wxID_OK);  // Close the dialog with ID_OK
        }

        // Cancel button event handler
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

        // wxString for GUI compatibility
        wxString name;
        wxString description;
        wxString frequency;
        wxString estimated_time;
        wxString notes;
        wxString location;

        // Vector of wxString for GUI compatibility
        vector<wxString> tags;
        vector<wxString> tools_required;
        vector<wxString> materials_needed;
        vector<wxString> days;
        // Adding callback function for status change
        function<void()> onUpdate;

        // Enumerations for difficulty, status, and priority
        DIFFICULTY difficulty;
        STATUS status;
        PRIORITY priority;

    public:
        // Constructor to initialize the Chore object
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
        virtual void startChore(wxWindow* parent) {
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
        virtual void completeChore(wxWindow* parent) {
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
        virtual void resetChore(wxWindow* parent) {
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
        virtual json toJSON() const {
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
        virtual wxString PrettyPrintClassAttributes() const {
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

        wxString getName() const {
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

        // Operator overloading for equality comparison
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
        // Comparison operators for sorting earnings
        struct CompareEarnings {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getEarnings() < b->getEarnings();
            }
        };
        // Comparison operators for sorting difficulty
        struct CompareDifficulty {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getDifficulty() < b->getDifficulty(); // Make sure Difficulty is comparable
            }
        };
        // Comparison operators for sorting ID
        struct CompareID {
            bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
                return a->getId() < b->getId();
            }
        };

    protected:
        // Helper function to format a vector of strings for display
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
    //*************************************************************************************************************************
  // Create the EasyChore class
    // Inherits from the Chore class
    class EasyChore : public Chore {
    private:
      wxString multitasking_tips; // Stores tips for multitasking while performing the chore

    public:
      // Constructor initializes EasyChore with JSON data, calling the base Chore constructor
      EasyChore(const json& j) : Chore(j) {
        // Retrieves multitasking tips from JSON and initializes the wxString
        multitasking_tips = j["multitasking_tips"].is_null() ? wxT("") : wxString(j["multitasking_tips"].get<std::string>());
      }

      // Override the startChore function for GUI-specific actions
      void startChore(wxWindow* parent) override {
        // Display a start message using a GUI message box
        wxMessageBox(wxT("Starting easy chore: "), wxT("Chore Start"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class startChore to perform any additional actions
        Chore::startChore(parent);
        // Display multitasking tips in a message box
        wxMessageBox(wxT("Multitasking Tips: ") + multitasking_tips, wxT("Tips"), wxOK | wxICON_INFORMATION, parent);
      }

      // Override the completeChore function for GUI-specific actions
      void completeChore(wxWindow* parent) override {
        // Display a completion message using a GUI message box
        wxMessageBox(wxT("Completing easy chore: "), wxT("Chore Completion"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class completeChore to perform any additional actions
        Chore::completeChore(parent);
      }

      // Override the resetChore function for GUI-specific actions
      void resetChore(wxWindow* parent) override {
        // Call the base class resetChore to reset chore status
        Chore::resetChore(parent);
      }

      // Override to provide a string representation of class attributes for debugging and display
      wxString PrettyPrintClassAttributes() const override {
        // Combine base class attributes with multitasking tips
        return Chore::PrettyPrintClassAttributes() +
          wxT("\nMultitasking Tips: ") + multitasking_tips;
      }

      // Override to serialize object data to JSON
      json toJSON() const override {
        // Start with base class JSON
        json j = Chore::toJSON();
        // Add multitasking tips to JSON
        j["multitasking_tips"] = std::string(multitasking_tips.mb_str());
        return j;
      }
    };

    //*************************************************************************************************************************
  // Create the MediumChore class
    // Inherits from the Chore class
    class MediumChore : public Chore {
    private:
      vector<wxString> variations; // Stores variations of the medium chore

    public:
      // Constructor initializes MediumChore with JSON data, calling the base Chore constructor
      MediumChore(const json& j) : Chore(j) {
        // Retrieves variations from JSON and initializes the vector of wxStrings
        variations = j["variations"].is_null() ? vector<wxString>() : parseVectorWXString(j["variations"]);
      }

      // Override the startChore function for GUI-specific actions
      void startChore(wxWindow* parent) override {
        // Display a start message using a GUI message box
        wxMessageBox(wxT("Starting medium chore: "), wxT("Chore Start"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class startChore to perform any additional actions
        Chore::startChore(parent);
        // Prepare and display variations available as a formatted string
        wxString variationStr = formatVector(variations);
        wxMessageBox(wxT("Variations available: ") + variationStr, wxT("Variations"), wxOK | wxICON_INFORMATION, parent);
      }

      // Override the completeChore function for GUI-specific actions
      void completeChore(wxWindow* parent) override {
        // Display a completion message using a GUI message box
        wxMessageBox(wxT("Completing medium chore: "), wxT("Chore Completion"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class completeChore to perform any additional actions
        Chore::completeChore(parent);
      }

      // Override the resetChore function for GUI-specific actions
      void resetChore(wxWindow* parent) override {
        // Call the base class resetChore to reset chore status
        Chore::resetChore(parent);
      }

      // Override to provide a string representation of class attributes for debugging and display
      wxString PrettyPrintClassAttributes() const override {
        // Combine base class attributes with chore variations
        return Chore::PrettyPrintClassAttributes() +
          wxT("\nVariations: ") + formatVector(variations);
      }

      // Override to serialize object data to JSON
      json toJSON() const override {
        // Start with base class JSON
        json j = Chore::toJSON();
        // Convert variations to a vector of std::string for JSON serialization
        std::vector<std::string> variationsStr;
        for (const auto& v : variations) {
          variationsStr.push_back(std::string(v.mb_str()));
        }
        j["variations"] = variationsStr;
        return j;
      }
    };

    //*************************************************************************************************************************
  // Create the HardChore class
    // Inherits from the Chore class
    class HardChore : public Chore {
    private:
      struct Subtask {
        wxString name;               // Name of the subtask
        wxString estimated_time;     // Estimated time to complete the subtask
        int earnings;                // Earnings from the subtask

        // Constructor initializes Subtask with JSON data
        Subtask(const json& subtaskJson) :
          name(subtaskJson["name"].is_null() ? wxT("") : wxString(subtaskJson["name"].get<std::string>())),
          estimated_time(subtaskJson["estimated_time"].is_null() ? wxT("") : wxString(subtaskJson["estimated_time"].get<std::string>())),
          earnings(subtaskJson["earnings"].is_null() ? 0 : subtaskJson["earnings"].get<int>()) {}
      };

      vector<Subtask> subtasks; // Stores subtasks associated with the hard chore

    public:
      // Constructor initializes HardChore with JSON data, calling the base Chore constructor
      HardChore(const json& j) : Chore(j) {
        // Populate subtasks vector from JSON array of subtasks
        for (const auto& subtaskJson : j["subtasks"].is_array() ? j["subtasks"] : json::array()) {
          subtasks.push_back(Subtask(subtaskJson));
        }
      }

      // Override the startChore function for GUI-specific actions
      void startChore(wxWindow* parent) override {
        // Display a start message using a GUI message box
        wxMessageBox(wxT("Starting hard chore: "), wxT("Chore Start"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class startChore to perform any additional actions
        Chore::startChore(parent);
        // Display subtask details using a message box
        wxString subtaskDetails;
        for (const auto& subtask : subtasks) {
          subtaskDetails += wxT("Subtask: ") + subtask.name + wxT(", Time: ") + subtask.estimated_time + wxT(", Earnings: $") + wxString::Format(wxT("%d"), subtask.earnings) + wxT("\n");
        }
        wxMessageBox(subtaskDetails, wxT("Subtask Details"), wxOK | wxICON_INFORMATION, parent);
      }

      // Override the completeChore function for GUI-specific actions
      void completeChore(wxWindow* parent) override {
        // Display a completion message using a GUI message box
        wxMessageBox(wxT("Completing hard chore: "), wxT("Chore Completion"), wxOK | wxICON_INFORMATION, parent);
        // Call the base class completeChore to perform any additional actions
        Chore::completeChore(parent);
      }

      // Override the resetChore function for GUI-specific actions
      void resetChore(wxWindow* parent) override {
        // Call the base class resetChore to reset chore status
        Chore::resetChore(parent);
      }

      // Override to provide a string representation of class attributes for debugging and display
      wxString PrettyPrintClassAttributes() const override {
        wxString result = Chore::PrettyPrintClassAttributes();
        for (const auto& subtask : subtasks) {
          result += wxT("\nSubtask: ") + subtask.name + wxT(", Time: ") + subtask.estimated_time + wxT(", Earnings: $") + wxString::Format(wxT("%d"), subtask.earnings);
        }
        return result;
      }

      // Override to serialize object data to JSON
      json toJSON() const override {
        json j = Chore::toJSON();
        json subtasksJson = json::array();
        for (const auto& subtask : subtasks) {
          json stJson;
          stJson["name"] = std::string(subtask.name.mb_str());
          stJson["estimated_time"] = std::string(subtask.estimated_time.mb_str());
          stJson["earnings"] = subtask.earnings;
          subtasksJson.push_back(stJson);
        }
        j["subtasks"] = subtasksJson;
        return j;
      }
    };

    //*************************************************************************************************************************
  // Create the Templated Container class
    template<typename T>
    class Container {
    private:
      vector<shared_ptr<T>> items;

    public:
      // Generic bubble sort using comparator
      template<typename Comparator>
      void sortItems(Comparator comp, bool ascending = true) {
        try {
          bool swapped;
          do {
            swapped = false;
            for (size_t i = 1; i < items.size(); i++) {
              if ((ascending && comp(items[i - 1], items[i])) || (!ascending && comp(items[i], items[i - 1]))) {
                swap(items[i - 1], items[i]);
                swapped = true;
              }
            }
          } while (swapped);
        }
        catch (const exception& e) {
          wxMessageBox(wxString("Exception thrown in sortItems: ") + wxString(e.what()), wxT("Error"), wxOK | wxICON_ERROR);
        }
      }

      void moveItemToAnotherContainer(int id, Container<T>& destination) {
        bool found = false;
        for (auto it = items.begin(); it != items.end(); ++it) {
          if ((*it)->getID() == id) {
            destination.items.push_back(*it);  // Add to destination
            wxMessageBox(wxString("Chore moved successfully: ") + (*it)->getName(), wxT("Info"), wxOK | wxICON_INFORMATION);
            items.erase(it);  // Remove from source
            found = true;
            break;  // Stop after finding and moving the item
          }
        }

        if (!found) {
          wxMessageBox(wxT("Chore not found."), wxT("Info"), wxOK | wxICON_INFORMATION);
        }
      }

      void deleteItem(int id) {
        auto it = find_if(items.begin(), items.end(), [id](const shared_ptr<T>& item) { return item->getID() == id; });
        if (it != items.end()) {
          items.erase(it);
          wxMessageBox(wxString("Chore deleted successfully."), wxT("Info"), wxOK | wxICON_INFORMATION);
        }
        else {
          wxMessageBox(wxT("Chore not found."), wxT("Error"), wxOK | wxICON_ERROR);
        }
      }

      void push_back(const shared_ptr<T>& item) {
        items.push_back(item);
      }

      size_t size() const {
        return items.size();
      }

      shared_ptr<T>& operator[](size_t index) {
        return items[index];
      }

      void display(wxWindow* parent) const {
        wxString info;
        for (const auto& item : items) {
          info += wxString("Chore: ") + item->getName() + " (ID: " + wxString::Format(wxT("%d"), item->getID()) + ")\n";
        }
        wxMessageBox(info, wxT("Container Items"), wxOK | wxICON_INFORMATION, parent);
      }

      bool empty() const {
        return items.empty();
      }

      void clear() {
        items.clear();
      }

      const vector<shared_ptr<T>>& getItems() const {
        return items;
      }

      auto begin() -> decltype(items.begin()) {
        return items.begin();
      }

      auto end() -> decltype(items.end()) {
        return items.end();
      }

      auto begin() const -> decltype(items.begin()) const {
        return items.begin();
      }

      auto end() const -> decltype(items.end()) const {
        return items.end();
      }
    };


    //*************************************************************************************************************************
  // Create the ChoreDoer class
    class ChoreDoer {
    public:
        Container<Chore> assignedChores;
        wxString name;
        int choreAmount;
        int age;
        int totalEarnings;

        // Constructor to initialize the ChoreDoer object
        ChoreDoer(const wxString& name, int age) : name(name), age(age), choreAmount(0), totalEarnings(0) {}

        // Method to assign a chore to the ChoreDoer
        void assignChore(const shared_ptr<Chore>& chore) {
            assignedChores.push_back(chore);
            choreAmount++;
        }

        wxString getName() const {
            return name;
        }

        int getTotalEarnings() const {
            return totalEarnings;
        }

        int getChoreAmount() const {
            return choreAmount;
        }

        // Method to display the ChoreDoer details
        wxString printChoreDoer() const {
            wxString result = "Chore Doer: " + name + "\n";
            result += "Total Earnings: $" + wxString::Format(wxT("%d"), totalEarnings) + "\n";
            result += "Chore Amount: " + wxString::Format(wxT("%d"), choreAmount) + "\n";
            return result;
        }

        // method to start a chore
        void startChore(int choreId, wxWindow* parent) {
            for (auto& chore : assignedChores) {
                if (chore->getId() == choreId && (chore->getStatus() == STATUS::NOT_STARTED)) {
                    chore->startChore(parent);
                    return;
                }
            }
        }


        // method to complete a chore
        void completeChore(int choreId, wxWindow* parent) {
            for (auto& chore : assignedChores) {
                if (chore->getId() == choreId && (chore->getStatus() == STATUS::IN_PROGRESS || chore->getStatus() == STATUS::NOT_STARTED)) {
                    chore->completeChore(parent);
                    totalEarnings += chore->getEarnings();
                    wxMessageBox(wxString::Format("Chore %s completed. Total earnings now: $%d", chore->getName(), totalEarnings), "Chore Completed", wxOK | wxICON_INFORMATION, parent);
                    break;
                }
                else {
                    wxMessageBox("Chore is already completed or not started.", "No Action Taken", wxOK | wxICON_INFORMATION, parent);
                }
            }
        }


        // method to reset a chore
        void resetChore(int choreId, wxWindow* parent) {
            for (auto& chore : assignedChores) {
                if (chore->getId() == choreId && (chore->getStatus() == STATUS::COMPLETED || chore->getStatus() == STATUS::IN_PROGRESS)) {
                    chore->resetChore(parent);
                    wxMessageBox(wxString::Format("Resetting Chore: %s", chore->getName()), "Chore Reset", wxOK | wxICON_INFORMATION, parent);
                }
                else {
                    wxMessageBox("Chore is already in the initial state (Not Started) or does not exist.", "Reset Unnecessary", wxOK | wxICON_INFORMATION, parent);
                }
            }
        }
    };

    //*********************************************************************************************************************
    // create the ChoreManager class
    class ChoreManager {
    private:
        json j;
        Container<Chore> chores;
        vector<shared_ptr<ChoreDoer>> doers;
        wxString dynamicFile;
        Client* client = nullptr;  // Initialize to nullptr to clearly indicate no client initially
        int choreCount;

    public:
        // Constructor to initialize the ChoreManager object
        ChoreManager(const wxString& fileName) : dynamicFile(fileName) {
            loadData();
        }
        // Destructor to save data when the object is destroyed
        ~ChoreManager() {
            saveData();
        }

        // Method to load data from the JSON file
        void loadData() {
            std::ifstream file(dynamicFile.ToStdString());
            if (!file) {
                wxMessageBox("Error opening file: " + dynamicFile, "File Error", wxOK | wxICON_ERROR);
                j = json::object(); // Initialize an empty JSON object if file fails to open
                return;
            }

            try {
                j = json::parse(file);
            }
            catch (const json::parse_error& e) {
                wxMessageBox("JSON Parsing Error: " + wxString(e.what()), "JSON Error", wxOK | wxICON_ERROR);
                j = json::object(); // Initialize an empty JSON object if parsing fails
            }
            file.close();
            // Load client from JSON
            if (j.contains("user_profile") && !j["user_profile"].is_null()) {
                auto userProfile = j["user_profile"];
                wxString username = userProfile.value("username", "defaultUser");
                wxString theme = userProfile.value("theme", "light");
                bool notify = userProfile.value("notify", false);
                client = new Client(username, theme, notify);
            }
            else {
                client = new Client("defaultUser", "light", false);
            }
            // call loadChores method
            loadChores();
        }

        // Method to load chores from the JSON file
        void loadChores() {
          try {
            if (j.contains("chores") && j["chores"].is_array()) {
              for (auto& choreJson : j["chores"]) {
                string difficulty = choreJson.value("difficulty", "unknown");
                shared_ptr<Chore> chore;

                if (difficulty == "easy") {
                  chore = make_shared<EasyChore>(choreJson);
                }
                else if (difficulty == "medium") {
                  chore = make_shared<MediumChore>(choreJson);
                }
                else if (difficulty == "hard") {
                  chore = make_shared<HardChore>(choreJson);
                }
                else {
                  cerr << "Unknown difficulty level: " << difficulty << endl;
                  continue;
                }

                chores.push_back(chore); // Adding to the container
                choreCount++;
              }
            }
          }
          catch (const json::parse_error& e) {
            cerr << "JSON parse error: " << e.what() << endl;
          }
          catch (const json::out_of_range& e) {
            cerr << "JSON out of range error: " << e.what() << endl;
          }
          catch (const json::type_error& e) {
            cerr << "JSON type error: " << e.what() << endl;
          }
          catch (const exception& e) {
            cerr << "Standard exception: " << e.what() << endl;
          }
        }

        // Method to load chore doers from the JSON file
        void addChoreDoer(const wxString& name, int age) {
            auto doer = std::make_shared<ChoreDoer>(name, age);
            doers.push_back(doer);
        }

        // Method to assign a chore to a ChoreDoer
        void assignChoreDoer(int choreId, const wxString& doerName) {
            auto chore = find_if(chores.begin(), chores.end(), [choreId](const shared_ptr<Chore>& c) {
                return c->getId() == choreId;
                });
            if (chore != chores.end()) {
                auto doer = find_if(doers.begin(), doers.end(), [doerName](const shared_ptr<ChoreDoer>& d) {
                    return d->getName() == doerName;
                    });

                if (doer != doers.end()) {
                    (*doer)->assignChore(*chore);
                }
                else {
                    wxLogError("ChoreDoer %s not found.", doerName);
                }
            }
            else {
                wxLogError("Chore ID %d not found.", choreId);
            }
        }

        // Method to display the ChoreDoer details
        wxString displayAssignedChores(const wxString& doerName) {
            auto doer = std::find_if(doers.begin(), doers.end(), [&doerName](const std::shared_ptr<ChoreDoer>& d) {
                return d->name == doerName;
                });

            if (doer != doers.end()) {
                return (*doer)->printChoreDoer();
            }

            return "Chore Doer not found.";
        }


        // Method to add a new chore to the list
        void addChore(const json& choreJson) {
            shared_ptr<Chore> chore;
            if (!choreJson.is_null()) {
              if (choreJson["difficulty"] == "easy") {
                chore = make_shared<EasyChore>(choreJson);
              }
              else {

              }
                auto chore = std::make_shared<Chore>(choreJson);
                chores.push_back(chore);
                saveData();  // Save every time a chore is added
            }
            else {
                wxMessageBox("Error adding chore: Invalid JSON", "JSON Error", wxOK | wxICON_ERROR);
            }
        }
        // saveData method to save the data to the JSON file
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
            if (file) {
                file << std::setw(4) << j << std::endl;
            }
            else {
                wxMessageBox("Error saving file: " + dynamicFile, "File Error", wxOK | wxICON_ERROR);
            }
            file.close();
        }

        // Method to display the client profile
        bool userExists(const wxString& username) {
            return j["user_profiles"].find(username.ToStdString()) != j["user_profiles"].end();
        }

        // Method to create a new user profile
        void createUser(const wxString& username, const wxString& theme, bool notify) {
            json newUser = {
                {"username", username.ToStdString()},
                {"theme", theme.ToStdString()},
                {"notify", notify}
            };
            j["user_profiles"][username.ToStdString()] = newUser;
            saveData();
        }

        // Method to update the user profile
        void updateUser(const wxString& username, const wxString& theme, bool notify) {
            j["user_profiles"][username.ToStdString()] = {
                {"username", username.ToStdString()},
                {"theme", theme.ToStdString()},
                {"notify", notify}
            };
            saveData();
        }

        // don't think we need this
        bool validateUser(wxString w, wxString z) {
            return true; // Placeholder for actual validation logic
        }

        // Method to display the chore list
        //******************************************************************
        //CHANGED DISPLAY CHORES TO WORK WITH WXWIDGETS (void function not allowed)
        vector<shared_ptr<Chore>> displayChores()
        {
            vector<shared_ptr<Chore>> choreList;
            for (const auto& chore : chores)
            {
                choreList.push_back(chore);
            }
            return choreList;
        }
        //void displayChores() {
        //    wxString info;
        //    for (const auto& chore : chores) {
        //        info += "Chore: " + chore->getName() + " (ID: " + wxString::Format(wxT("%d"), chore->getId()) + ")\n";
        //    }
        //    wxMessageBox(info, "Chore List", wxOK | wxICON_INFORMATION);
        //}

    };
    //*******************************************
    //CREATED CHORES FRAME TO DEAL WITH WHEN VIEW CHORE IS SELECTED
    //this frame is called in event.getid == 1... will display clickable chores
    class ChoresFrame : public wxFrame {
    public:
        ChoresFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    private:
        void OnChoreSelected(wxCommandEvent& event);
    };

    ChoresFrame::ChoresFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(NULL, wxID_ANY, title, pos, size) {
        ChoreManager choreManager(DATA_FILE_PATH + "data.json");
        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
        wxChoice* choice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, NULL, 0);

        choice->Append("Select Chore");

        // Loading chores into the choice menu
        for (const auto& chore : choreManager.displayChores()) {
            choice->Append(chore->getName());
        }

        sizer->Add(choice, 1, wxEXPAND | wxALL, 5);
        SetSizerAndFit(sizer);

        //even handler for the clickable menu
        choice->Bind(wxEVT_CHOICE, &ChoresFrame::OnChoreSelected, this);
    }
    //while in the clickablbe menu.. here is where more information will be displayed on click
    // im thinking .... description... earnings...difficulty? maybe 
    void ChoresFrame::OnChoreSelected(wxCommandEvent& event) {
        int selection = event.GetInt();
        //each selection corresponds with its index.. 0 is Select chore.. and so on
        if (selection == 0) {
            //just a test message to verify it worked
            wxMessageBox("this worked", "chore 1", wxOK | wxICON_INFORMATION);
            return;
        }

        //chore selected displayed
        wxString selectedChore = event.GetString();
        wxMessageBox("Selected Chore: " + selectedChore, "Chore Selected", wxOK | wxICON_INFORMATION);
    }

    //**********************************************************************************************
        // Create the subclass of wxFrame
        // MyFrame serves as the main frame for the application
    class MyFrame : public wxFrame {
    public:
        // Constructor to initialize the frame
        MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    private:
        void CreateMenu();                // Method to create the menu
        void CreateControls();               // Method to create the controls
        void OnExit(wxCommandEvent& event);    // Event handler for the Exit menu item
        void OnAbout(wxCommandEvent& event);     // Event handler for the About menu item
        void OnButtonClick(wxCommandEvent& event);   // Event handler for the button click
        //******************************************
        void OnClick(wxCommandEvent& event); //onclick added
    };
    //**************************************************
    //ID_SORT_CHORES will be an enum
    //ID_SEARCH_CHORES will be an enum
    enum
    {
        ID_VIEW_CHORES = 1,     // 1 will be passed in void call to onclick
        ID_SORT_CHORES = 2,
        ID_SEARCH_CHORES = 3
    };
    // Create the subclass of wxApp
    MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
        : wxFrame(nullptr, wxID_ANY, title, pos, size) {
        CreateMenu();
        CreateControls();
    }
    // Method to create the menu
    void MyFrame::CreateMenu() {
        // Creating menu items
        wxMenu* menuFile = new wxMenu;
        menuFile->Append(wxID_EXIT, "Exit");

        wxMenu* menuHelp = new wxMenu;
        menuHelp->Append(wxID_ABOUT, "About");
        //making a chore menu
        wxMenu* menuChore = new wxMenu;
        menuChore->Append(wxID_ANY, "Add Chore");
        //****************************************************
        menuChore->Append(ID_VIEW_CHORES, "View Chores"); // changed from wxID_ANY to ID_VIEW so OnClick will associate with it.
        menuChore->Append(wxID_ANY, "Edit Chore");

        // Adding menus to the menu bar
        wxMenuBar* menuBar = new wxMenuBar;
        menuBar->Append(menuFile, "File");
        menuBar->Append(menuHelp, "Help");
        menuBar->Append(menuChore, "Chore");

        // Setting the menu bar to the frame
        SetMenuBar(menuBar);

        // Binding events to their handlers
        Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
        Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
        //Binding control click**********************************************
        //bind for ID_VIEW_CHORES
        Bind(wxEVT_MENU, &MyFrame::OnClick, this, ID_VIEW_CHORES);
    }
    // Method to create the controls
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

    // Event handler for the Exit menu item
    void MyFrame::OnExit(wxCommandEvent& event) {
        Close(true);  // Close the frame
    }
    // Event handler for the About menu item
    void MyFrame::OnAbout(wxCommandEvent& event) {
        wxMessageBox("This is a wxWidgets application for managing chores.", "About Chore Manager", wxOK | wxICON_INFORMATION);
    }

    // Event handler for the button click
    void MyFrame::OnButtonClick(wxCommandEvent& event) {
        wxMessageBox("Button clicked!", "Event", wxOK | wxICON_INFORMATION);
    }

    //when menu item is clicked**********************************************
    //will make a sort and search that aligns with getid() 2  and 3
    void MyFrame::OnClick(wxCommandEvent& event)
    {

        if (event.GetId() == 1)
        {
            //this transfers control to the frame created ChoresFrame....it has its own binding event for when a chore is clicked
            ChoresFrame* choresFrame = new ChoresFrame("SELECT CHORE", wxDefaultPosition, wxSize(300, 200));
            choresFrame->Show(true);

        }
        else if (event.GetId() == 2)
        {
            //SORT CHORES USED 
        }
        else if (event.GetId() == 3)
        {
            //SEARCH CHORES USED
        }
    }
    // Create the subclass of wxApp
    // ChoreApp serves as the application object
    class ChoreApp : public wxApp {
    public:
        virtual bool OnInit() {
            // Load user profile from the JSON file

            ChoreManager choreManager(DATA_FILE_PATH + "data.json");   // Create a ChoreManager object and path to the JSON file
            LoginDialog* loginFrame = new LoginDialog(nullptr, &choreManager);     // Create a login dialog
            loginFrame->ShowModal();                                            // Show the login dialog

            // Create and show the main frame
            MyFrame* frame = new MyFrame("Chore Manager", wxPoint(50, 50), wxSize(450, 340));
            frame->Show(true);
            return true;
        }

        // Update in ChoreApp to handle user login and new user creation
        class LoginDialog : public wxDialog {
        public:
            // Constructor to initialize the login dialog
            LoginDialog(wxWindow* parent, ChoreManager* choreManager)
                : wxDialog(parent, wxID_ANY, wxT("Login or Register"), wxDefaultPosition, wxDefaultSize),
                m_choreManager(choreManager) {
                // Setup UI components for login
                auto* sizer = new wxBoxSizer(wxVERTICAL);
                auto* usernameLabel = new wxStaticText(this, wxID_ANY, wxT("Username:"));
                m_usernameText = new wxTextCtrl(this, wxID_ANY);
                auto* themeLabel = new wxStaticText(this, wxID_ANY, wxT("Choose theme:"));
                // Theme choices
                wxArrayString themeChoices;
                themeChoices.Add("light");
                themeChoices.Add("dark");
                m_themeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, themeChoices);
                m_themeChoice->SetSelection(0);  // Default to 'light'
                // Notification checkbox
                auto* notifyLabel = new wxStaticText(this, wxID_ANY, wxT("Enable notifications:"));
                m_notifyCheckBox = new wxCheckBox(this, wxID_ANY, wxT(""));

                // Login button
                auto* loginButton = new wxButton(this, wxID_OK, wxT("Login or Register"));

                // Add components to sizer
                sizer->Add(usernameLabel, 0, wxALL, 5);
                sizer->Add(m_usernameText, 0, wxEXPAND | wxALL, 5);
                sizer->Add(themeLabel, 0, wxALL, 5);
                sizer->Add(m_themeChoice, 0, wxEXPAND | wxALL, 5);
                sizer->Add(notifyLabel, 0, wxALL, 5);
                sizer->Add(m_notifyCheckBox, 0, wxALL, 5);
                sizer->Add(loginButton, 0, wxALIGN_CENTER | wxALL, 5);

                // Set sizer for the dialog
                SetSizer(sizer);

                // Bind login button click event
                loginButton->Bind(wxEVT_BUTTON, &LoginDialog::OnLogin, this);
            }


        private:
            ChoreManager* m_choreManager;       // Pointer to the ChoreManager object
            wxTextCtrl* m_usernameText;         // Text control for username
            wxChoice* m_themeChoice;             // Choice control for theme
            wxCheckBox* m_notifyCheckBox;        // Checkbox for notifications

            // Event handler for the login button
            void OnLogin(wxCommandEvent& event) {
                wxString username = m_usernameText->GetValue();
                wxString theme = m_themeChoice->GetStringSelection();
                bool notify = m_notifyCheckBox->IsChecked();

                if (!m_choreManager->userExists(username)) {
                    m_choreManager->createUser(username, theme, notify);
                    wxMessageBox("New user created: " + username + ". Welcome to Chore Manager!", "User Created", wxOK | wxICON_INFORMATION);
                }
                else {
                    m_choreManager->updateUser(username, theme, notify);
                    wxMessageBox("Welcome back, " + username + "!", "Login Success", wxOK | wxICON_INFORMATION);
                }

                // Close the dialog
                EndModal(wxID_OK);
            }
        };
    };

}

wxIMPLEMENT_APP(ChoreAppNamespace::ChoreApp);  // This macro creates the main() function for the application