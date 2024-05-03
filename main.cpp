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
#include <algorithm>
#include <random>
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
      struct Preferences {
        bool notify;
        wxString theme;  // Using wxString for better integration with wxWidgets

        Preferences(const json& j)
          : notify(j.contains("notify") && !j["notify"].is_null() ? j["notify"].get<bool>() : false),
          theme(j.contains("theme") && !j["theme"].is_null() ? wxString(j["theme"].get<string>()) : wxString("Default")) {}

        // Validate theme input
        static wxString validateTheme(const wxString& theme) {
          if (theme == "dark" || theme == "light") {
            return theme;
          }
          wxLogWarning("Invalid theme specified: %s. Setting default to 'light'.", theme);
          return "light";  // Default theme
        }
      };

      struct UserProfile {
        wxString username;
        wxString last_logged_in;
        Preferences preferences;

        UserProfile(const json& j)
          : username(j.contains("username") && !j["username"].is_null() ? wxString(j["username"].get<string>()) : wxString("DefaultUser")),
          last_logged_in(getCurrentDateTime()),
          preferences(j.contains("preferences") && !j["preferences"].is_null() ? j["preferences"] : json{}) {}

        wxString getCurrentDateTime() {
          wxDateTime now = wxDateTime::Now();
          return now.FormatISOCombined(' ');
        }
      };

      UserProfile userProfile;

    public:
      Client(const json& j)
        : userProfile(j.contains("user_profile") ? j["user_profile"] : json{}) {}

      json toJSON() const {
        // Create a json object
        json output;

        // Add username to the json output, check if it's empty and provide a default value
        output["username"] = userProfile.username.IsEmpty() ? "Unknown" : userProfile.username.ToStdString();

        // Add last_logged_in to the json output, check if it's empty and provide a default value
        output["last_logged_in"] = userProfile.last_logged_in.IsEmpty() ? "Never" : userProfile.last_logged_in.ToStdString();

        // Add preferences to the json output, handling notify and theme with defaults if they are empty
        output["preferences"] = {
            {"notify", userProfile.preferences.notify},  // bool value does not need a null check
            {"theme", userProfile.preferences.theme.IsEmpty() ? "Default" : userProfile.preferences.theme.ToStdString()}
        };

        return output;
      }

      void modifyProfile(wxString &newName, bool &newNotify, wxString &newTheme) {
        wxMessageDialog(nullptr, wxT("Modifying Profile...")).ShowModal();

        setUsername(newName);
        setNotify(newNotify);
        setTheme(newTheme);

        userProfile.last_logged_in = userProfile.getCurrentDateTime();
        
        // Assuming JSON input for preferences needs custom handling or different dialogs
        // This part might need further adaptation based on actual preferences structure and UI design
      }

      wxString printProfile() const {
        wxString result;
        result += "Username: " + userProfile.username + "\n";
        result += "Last Logged In: " + userProfile.last_logged_in + "\n";
        result += "Notifications: " + wxString(userProfile.preferences.notify ? "Enabled" : "Disabled") + "\n";
        result += "Theme: " + userProfile.preferences.theme + "\n";
        return result;
      }

      // Setters and getters for username
      void setUsername(const wxString& newUsername) {
        if (!newUsername.IsEmpty()) {
          userProfile.username = newUsername;
          userProfile.last_logged_in = userProfile.getCurrentDateTime();  // Update last logged in when username changes
        }
      }

      // Setters and getters for notification settings
      void setNotify(bool newNotify) {
        userProfile.preferences.notify = newNotify;
      }


      // Setters and getters for theme
      void setTheme(const wxString& newTheme) {
        userProfile.preferences.theme = userProfile.preferences.validateTheme(newTheme);
      }

      wxString getUserName() const {
        return userProfile.username;
      }

      wxString getLastLoggedIn() const {
        return userProfile.last_logged_in;
      }

      wxString getNotify() const {
        return userProfile.preferences.notify ? wxT("Enabled") : wxT("Disabled");
      }

      wxString getTheme() const {
        return userProfile.preferences.theme.IsEmpty() ? wxString("Default") : userProfile.preferences.theme;
      }

      void toggleNotify() {
        userProfile.preferences.notify = !userProfile.preferences.notify;
      }

      void toggleTheme() {
        if (userProfile.preferences.theme == "dark") {
          userProfile.preferences.theme = "light";
        }
        else {
          userProfile.preferences.theme = "dark";
        }
      }
    };

    //*******************************************************************************************************************

    class ClientDialog : public wxDialog {
    public:
      ClientDialog(wxWindow* parent, Client* client)
        : wxDialog(parent, wxID_ANY, wxT("Edit Client Profile"), wxDefaultPosition, wxSize(350, 200)),
        m_client(client) {

        // Setup UI components
        auto* sizer = new wxBoxSizer(wxVERTICAL);
        m_usernameCtrl = new wxTextCtrl(this, wxID_ANY, m_client->getUserName());
        m_themeCtrl = new wxTextCtrl(this, wxID_ANY, m_client->getTheme());
        m_notifyCtrl = new wxCheckBox(this, wxID_ANY, wxT("Enable Notifications"), wxDefaultPosition, wxDefaultSize, wxCHK_2STATE);
        m_notifyCtrl->SetValue(m_client->getNotify() == "Enabled");

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
      wxTextCtrl* m_themeCtrl;     // Text control for theme
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
        Chore(const json& j, function<void()> callback = nullptr) : onUpdate(callback) {
            id = j["id"].is_null() ? -1 : j["id"].get<int>();
            name = j["name"].is_null() ? wxString("") : wxString(j["name"].get<string>());
            description = j["description"].is_null() ? wxString("") : wxString(j["description"].get<string>());
            frequency = j["frequency"].is_null() ? wxString("") : wxString(j["frequency"].get<string>());
            estimated_time = j["estimated_time"].is_null() ? wxString("") : wxString(j["estimated_time"].get<string>());
            earnings = j["earnings"].is_null() ? 0 : j["earnings"].get<int>();

            days = j["days"].is_null() ? vector<wxString>() : parseVectorWXString(j["days"]);
            location = j["location"].is_null() ? wxString("") : wxString(j["location"].get<string>());
            tools_required = j["tools_required"].is_null() ? vector<wxString>() : parseVectorWXString(j["tools_required"]);
            materials_needed = j["materials_needed"].is_null() ? vector<wxString>() : parseVectorWXString(j["materials_needed"]);
            notes = j["notes"].is_null() ? wxString("") : wxString(j["notes"].get<string>());
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
                    result.push_back(wxString(item.get<string>()));
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
                + "Name: " + name.ToStdString() + "\n"  // Convert wxString to string
                + "Description: " + description.ToStdString() + "\n"  // Convert wxString to string
                + "Frequency: " + frequency.ToStdString() + "\n"  // Convert wxString to string
                + "Estimated Time: " + estimated_time.ToStdString() + "\n"  // Convert wxString to string
                + "Earnings: " + to_string(earnings) + "\n"
                + "Days: " + formatVector(days) + "\n"  // Ensure formatVector returns string
                + "Location: " + location.ToStdString() + "\n"  // Convert wxString to string
                + "Tools Required: " + formatVector(tools_required) + "\n"  // Ensure formatVector returns string
                + "Materials Needed: " + formatVector(materials_needed) + "\n"  // Ensure formatVector returns string
                + "Notes: " + notes.ToStdString() + "\n"  // Convert wxString to string
                + "Tags: " + formatVector(tags) + "\n"  // Ensure formatVector returns string
                + "Status: " + toStringS(status) + "\n"  // Convert wxString to string if needed
                + "Priority: " + toStringP(priority) + "\n"  // Convert wxString to string if needed
                + "Difficulty: " + toStringD(difficulty);  // Convert wxString to string if needed

            return result;
        }

        wxString simplePrint() const {
          wxString result = "Chore ID: " + to_string(id) + "\n"
            + "Name: " + name.ToStdString() + "\n"  // Convert wxString to string
            + "Description: " + description.ToStdString() + "\n"  // Convert wxString to string
            + "Earnings: " + to_string(earnings) + "\n";

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


    // Global sorting functions for Chores:
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

    // Search criteria functions
    bool matchById(const shared_ptr<Chore>& chore, const int& id) {
      return chore->getId() == id;
    }

    bool matchByName(const shared_ptr<Chore>& chore, const wxString& name) {
      return chore->getName() == name;
    }

    bool matchByEarnings(const shared_ptr<Chore>& chore, const int& earnings) {
      return chore->getEarnings() == earnings;
    }

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
        multitasking_tips = j["multitasking_tips"].is_null() ? wxString("") : wxString(j["multitasking_tips"].get<string>());
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
        j["multitasking_tips"] = string(multitasking_tips.mb_str());
        return j;
      }

      wxString getMultitaskingTips() {
        return multitasking_tips;
      }

      void setMultitaskingTips(const wxString &newTip) {
        multitasking_tips = newTip;
        triggerUpdate();
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
        // Convert variations to a vector of string for JSON serialization
        vector<string> variationsStr;
        for (const auto& v : variations) {
          variationsStr.push_back(string(v.mb_str()));
        }
        j["variations"] = variationsStr;
        return j;
      }

      vector<wxString> getVariations() {
        return variations;
      }

      void setVariations(vector<wxString> newVariation) {
        variations = newVariation;
        triggerUpdate();
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
          name(subtaskJson["name"].is_null() ? wxString("") : wxString(subtaskJson["name"].get<string>())),
          estimated_time(subtaskJson["estimated_time"].is_null() ? wxString("") : wxString(subtaskJson["estimated_time"].get<string>())),
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
          stJson["name"] = string(subtask.name.mb_str());
          stJson["estimated_time"] = string(subtask.estimated_time.mb_str());
          stJson["earnings"] = subtask.earnings;
          subtasksJson.push_back(stJson);
        }
        j["subtasks"] = subtasksJson;
        return j;
      }

      // Getter function to retrieve the vector of subtasks
      const vector<Subtask>& getSubtasks() const {
        return subtasks;
      }

      // Setter function to set the vector of subtasks
      void setSubtasks(const vector<Subtask>& newSubtasks) {
        subtasks = newSubtasks;
      }

    };

    //*************************************************************************************************************************
  // Create the Templated Container class
// Template class definition for a Container that manages a list of shared_ptr to objects of type T.
    template<typename T>
    class Container {
    private:
      vector<shared_ptr<T>> items;  // Private vector to store the shared pointers of type T.

    public:
      // Function to sort the items within the container using a generic comparator.
      template<typename Comparator>
      void sortItems(Comparator comp, bool ascending = true) {
        try {
          bool swapped;  // Boolean flag to keep track of swapping during the sort.
          do {
            swapped = false;
            for (size_t i = 1; i < items.size(); i++) {
              // Condition to determine ascending or descending order and compare items accordingly.
              if ((ascending && comp(items[i - 1], items[i])) || (!ascending && comp(items[i], items[i - 1]))) {
                swap(items[i - 1], items[i]);  // Swaps two elements if they are out of order.
                swapped = true;  // Set swapped to true indicating a swap occurred.
              }
            }
          } while (swapped);  // Continue sorting until no swaps are made.
        }
        catch (const exception& e) {
          // Handles exceptions by showing a message box with the error.
          wxMessageBox(wxString("Exception thrown in sortItems: ") + wxString(e.what()), wxT("Error"), wxOK | wxICON_ERROR);
        }
      }

      // Templated search method
      template<typename Key>
      vector<shared_ptr<T>> searchItem(const Key& key, function<bool(const shared_ptr<T>&, const Key&)> matchCriteria) {
        vector<shared_ptr<T>> results;
        for (auto& item : items) {
          if (matchCriteria(item, key)) {
            results.push_back(item);
          }
        }
        return results;
      }

      // Function to display search results
      void displaySearchResults(const vector<shared_ptr<T>>& results) {
        if (results.empty()) {
          wxMessageBox("No items found.", "Search Results", wxOK | wxICON_INFORMATION);
        }
        else {
          wxString info;
          for (const auto& item : results) {
            info += item->simplePrint() + "\n";
          }
          wxMessageBox(info, "Search Results", wxOK | wxICON_INFORMATION);
        }
      }

      // Method to move an item from this container to another container by ID.
      void moveItemToAnotherContainer(int id, Container<T>& destination) {
        bool found = false;  // Flag to check if the item was found.
        for (auto it = items.begin(); it != items.end(); ++it) {
          if ((*it)->getID() == id) {
            destination.items.push_back(*it);  // Add the item to the destination container.
            wxMessageBox(wxString("Chore moved successfully: ") + (*it)->getName(), wxT("Info"), wxOK | wxICON_INFORMATION);
            items.erase(it);  // Remove the item from the current container.
            found = true;
            break;  // Exit loop once item is moved.
          }
        }

        if (!found) {
          // If no item is found, display an information message.
          wxMessageBox(wxT("Chore not found."), wxT("Info"), wxOK | wxICON_INFORMATION);
        }
      }

      // Method to delete an item from the container based on its ID.
      void deleteItem(int id) {
        auto it = std::find_if(items.begin(), items.end(), [id](const shared_ptr<T>& item) { return item->getId() == id; });
        if (it != items.end()) {
          items.erase(it);  // Remove the item if found.
          wxMessageBox(wxString("Chore deleted successfully."), wxT("Info"), wxOK | wxICON_INFORMATION);
        }
        else {
          // Display an error message if the item is not found.
          wxMessageBox(wxT("Chore not found."), wxT("Error"), wxOK | wxICON_ERROR);
        }
      }

      // Method to add an item to the container.
      void push_back(const shared_ptr<T>& item) {
        items.push_back(item);
      }

      // Returns the number of items in the container.
      size_t size() const {
        return items.size();
      }

      // Overloaded operator[] to access items by their index.
      shared_ptr<T>& operator[](size_t index) {
        return items[index];
      }

      // Display the details of all items in the container using a message box.
      // Displays all chores
      void displayAllItems(wxWindow* parent) const {
        wxString info;
        for (const auto& item : items) {
          info += wxString("Chore: ") + item->getName() + " (ID: " + wxString::Format(wxT("%d"), item->getId()) + ")\n";
        }
        wxMessageBox(info, wxT("Container Items"), wxOK | wxICON_INFORMATION, parent);
      }

      wxString returnAllItems() {
        wxString info;
        for (const auto& item : items) {
          info += wxString("Chore: ") + item->getName() + " (ID: " + wxString::Format(wxT("%d"), item->getId()) + ")\n";
        }
        return info;
      }

      // Check if the container is empty.
      bool empty() const {
        return items.empty();
      }

      // Clears all items from the container.
      void clear() {
        items.clear();
      }

      // Accessor for the underlying vector of items.
      const vector<shared_ptr<T>>& getItems() const {
        return items;
      }

      // Return iterators to enable range-based loops directly on the container.
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

        // Sort assigned chores by earnings
        void sortAssignedChoresByEarnings(bool ascending = true) {
          assignedChores.sortItems(CompareEarnings(), ascending);
        }

        // Sort assigned chores by difficulty
        void sortAssignedChoresByDifficulty(bool ascending = true) {
          assignedChores.sortItems(CompareDifficulty(), ascending);
        }

        // Sort assigned chores by ID
        void sortAssignedChoresByID(bool ascending = true) {
          assignedChores.sortItems(CompareID(), ascending);
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
                wxMessageBox("Chore is already completed or not started.", "No Action Taken", wxOK | wxICON_INFORMATION, parent);
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
        // Overloaded insertion operator to output the details of the ChoreDoer
        friend ostream& operator<<(ostream& os, const ChoreDoer& chDoer);
    };

    // Overloaded ostream operator to facilitate easy output of ChoreDoer's state
    ostream& operator<<(ostream& os, const ChoreDoer& chDoer) {
      os << chDoer.printChoreDoer();  // Output formatted chore doer details
      return os;
    }

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

        // Clear all data from ChoreManager
        void clearAll() {
          try {
            // Clear original Container of chores
            clearChores();
            // Clear assigned Container of chores for each chore doer
            clearAllAssignedChores();
            // Clear chore doers
            clearChoreDoers();
          }
          catch (const exception& e) {
            wxMessageBox(wxString::Format("Exception caught in clearAll: %s", e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }

        // Clear all chores from the container
        void clearChores() {
          try {
            if (!chores.empty()) {
              chores.clear();
            }
          }
          catch (const exception& e) {
            wxMessageBox(wxString::Format("Exception caught in clearChores: %s", e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }

        // Clear all chore doers from the list
        void clearChoreDoers() {
          try {
            doers.clear();
          }
          catch (const exception& e) {
            wxMessageBox(wxString::Format("Exception caught in clearChoreDoers: %s", e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }

        // Clear all assigned chores for each chore doer
        void clearAllAssignedChores() {
          try {
            if (!doers.empty()) {
              for (auto& doer : doers) {
                if (!doer->assignedChores.empty()) {
                  doer->assignedChores.clear();
                }
              }
            }
          }
          catch (const exception& e) {
            wxMessageBox(wxString::Format("Exception caught in clearAllAssignedChores: %s", e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }

    public:
        // Constructor to initialize the ChoreManager object
        ChoreManager(const wxString& fileName) : dynamicFile(fileName) {
            loadData();
        }
        // Destructor to save data when the object is destroyed
        ~ChoreManager() {
            saveData();
            clearAll();
        }

        // Method to load data from the JSON file
        void loadData() {
          ifstream file(dynamicFile.ToStdString());
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
            client = new Client(userProfile); //Will be saved user, or DefaultUser

            
          }
          // Data does not contain user_profile
          else {
            json defaultProfile;
            defaultProfile["user_profile"] = {
                {"username", "DefaultUser"},
                {"preferences", {{"theme", "light"}, {"notify", false}}}
            };

            client = new Client(defaultProfile);
          }

          // call loadChores method
          loadChores();
        }

        // Method to load chores from the JSON file and create them uniquely based on difficulty
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
                  wxMessageBox("Unknown difficulty level: " + difficulty, "Loading Chore Error", wxOK | wxICON_ERROR);
                  continue;
                }

                chores.push_back(chore); // Adding to the container
                choreCount++;
              }
            }
          }
          catch (const json::parse_error& e) {
            wxMessageBox(wxString::Format("JSON parse error: %s", e.what()), "Error", wxOK | wxICON_ERROR);
          }
          catch (const json::out_of_range& e) {
            wxMessageBox(wxString::Format("JSON out of range error: %s", e.what()), "Error", wxOK | wxICON_ERROR);
          }
          catch (const json::type_error& e) {
            wxMessageBox(wxString::Format("JSON type error: %s", e.what()), "Error", wxOK | wxICON_ERROR);
          }
          catch (const exception& e) {
            wxMessageBox(wxString::Format("Standard exception: %s", e.what()), "Error", wxOK | wxICON_ERROR);
          }
        }

        //Function wrapper for templated sort function
        template<typename Comparator>
        void sortChores(Comparator comp, bool ascending = true) {
          try
          {
            // Uses templated functions
            chores.sortItems(comp, ascending);
          }
          catch (const exception& e)
          {
            wxMessageBox("Exception caught in sortChores: " , e.what(), wxOK | wxICON_ERROR);
          }
        }

        

        // Create a NEW output file containing user_profile, chore_doers, 
        // chore doers name, and chore doers assigned chores
        void outputChoreAssignmentsToFile(const wxString& outputPath) {
          json output;

          // Include client's user profile data
          output["user_profile"] = client->toJSON();

          // Include chore assignments for each chore doer
          output["chore_doers"] = json::array();

          // Iterate through chore doers
          for (const auto& doer : doers) {
            json doerJson;
            doerJson["name"] = doer->getName().ToStdString();
            doerJson["chores"] = json::array();

            // Iterate through chore doers assigned chores
            for (const auto& chore : doer->assignedChores) {
              doerJson["chores"].push_back(chore->toJSON());
            }
            // Add doerJson json data to output json data (build json file)
            output["chore_doers"].push_back(doerJson);
          }

          // Convert JSON to string with pretty formatting
          string jsonData = output.dump(4);

          // Use standard C++ file handling
          ofstream outFile(outputPath.ToStdString());
          if (!outFile.is_open()) {
            wxMessageBox(wxT("Could not open file to write chore assignments."), "File Error", wxOK | wxICON_ERROR);
            return;
          }

          // Write the JSON data to the file with indentation
          outFile << setw(4) << jsonData;
          outFile.close();
        }

        void performSearchByID(int id) {
          auto resultsById = chores.searchItem<int>(id, matchById);
          if (resultsById.empty()) {
            // Search through each ChoreDoer's assigned chores
            for (auto& doer : doers) {
              auto doerResults = doer->assignedChores.searchItem<int>(id, matchById);
              if (!doerResults.empty()) {
                resultsById.insert(resultsById.end(), doerResults.begin(), doerResults.end());
              }
            }
          }
          if (resultsById.empty()) {
            wxMessageBox("No chore found with the specified ID.", "Search Results", wxICON_INFORMATION);
          }
          else {
            chores.displaySearchResults(resultsById);
          }
        }

        void searchByName(const wxString& name) {
          auto resultsName = chores.searchItem<wxString>(name, matchByName);
          if (resultsName.empty()) {
            // Search through each ChoreDoer's assigned chores
            for (auto& doer : doers) {
              auto doerResults = doer->assignedChores.searchItem<wxString>(name, matchByName);
              if (!doerResults.empty()) {
                resultsName.insert(resultsName.end(), doerResults.begin(), doerResults.end());
              }
            }
          }
          if (resultsName.empty()) {
            wxMessageBox("No chore found with the specified name.", "Search Results", wxICON_INFORMATION);
          }
          else {
            chores.displaySearchResults(resultsName);
          }
        }

        void searchByEarnings(int earnings) {
          auto resultsEarned = chores.searchItem<int>(earnings, matchByEarnings);
          if (resultsEarned.empty()) {
            // Search through each ChoreDoer's assigned chores
            for (auto& doer : doers) {
              auto doerResults = doer->assignedChores.searchItem<int>(earnings, matchByEarnings);
              if (!doerResults.empty()) {
                resultsEarned.insert(resultsEarned.end(), doerResults.begin(), doerResults.end());
              }
            }
          }
          if (resultsEarned.empty()) {
            wxMessageBox("No chore found with the specified earnings.", "Search Results", wxICON_INFORMATION);
          }
          else {
            chores.displaySearchResults(resultsEarned);
          }
        }

        // Method to sort chores by earnings
        void sortChoresByEarnings(bool ascending = true) {
          CompareEarnings comp;
          chores.sortItems(comp, ascending);
          wxMessageBox("Chores sorted by earnings.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        // Method to sort chores by difficulty
        void sortChoresByDifficulty(bool ascending = true) {
          CompareDifficulty comp;
          chores.sortItems(comp, ascending);
          wxMessageBox("Chores sorted by difficulty.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        // Method to sort chores by ID
        void sortChoresByID(bool ascending = true) {
          CompareID comp;
          chores.sortItems(comp, ascending);
          wxMessageBox("Chores sorted by ID.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        wxString searchChoreDoer(const wxString& name) {
          wxString info;
          for (auto doer : doers) {
            if (doer->getName() == name) {
              info += doer->printChoreDoer() + "\n";
              if (!doer->assignedChores.empty())
                info += displayAssignedChores(name);
            }
            else {
              info = "Could not find specified chore doer";
            }
          }

          return info;
        }

        // Sort all chore doers' assigned chores by earnings
        void sortAllChoreDoersChoresByEarnings(bool ascending = true) const {
          for (auto& doer : doers) {
            doer->sortAssignedChoresByEarnings(ascending);
          }
          wxMessageBox("Sorted all chore doers' chores by earnings.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        // Sort all chore doers' assigned chores by difficulty
        void sortAllChoreDoersChoresByDifficulty(bool ascending = true) const {
          for (auto& doer : doers) {
            doer->sortAssignedChoresByDifficulty(ascending);
          }
          wxMessageBox("Sorted all chore doers' chores by difficulty.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        // Sort all chore doers' assigned chores by ID
        void sortAllChoreDoersChoresByID(bool ascending = true) const {
          for (auto& doer : doers) {
            doer->sortAssignedChoresByID(ascending);
          }
          wxMessageBox("Sorted all chore doers' chores by ID.", "Sort Info", wxOK | wxICON_INFORMATION);
        }

        // Method to load chore doers from the JSON file
        void addChoreDoer(const wxString& name, int age) {
            auto doer = make_shared<ChoreDoer>(name, age);
            doers.push_back(doer);
        }

        // Method to delete a chore from the available chore list by ID
        void deleteChoreFromAvailable(int choreId) {
          chores.deleteItem(choreId);
        }

        // Method to delete a chore from any chore doer's assigned list by ID
        void deleteChoreFromAnyDoer(int choreId) const {
          for (auto& doer : doers) {
            doer->assignedChores.deleteItem(choreId);
          }
        }

        // Method to delete a chore doer from the system by name
        void deleteChoreDoer(const wxString& name) {
          try {
            auto it = std::find_if(doers.begin(), doers.end(), [&name](const auto& doer) {
              return doer->getName() == name;
              });

            if (it != doers.end()) {
              doers.erase(it);
              wxMessageBox(wxString("Chore doer removed successfully."), wxT("Info"), wxOK | wxICON_INFORMATION);
            }
            else {
              wxMessageBox(wxString("Chore doer not found."), wxT("Info"), wxOK | wxICON_INFORMATION);
            }
          }
          catch (const std::exception& e) {
            wxMessageBox(wxString("Exception thrown in deleteChoreDoer: ") + wxString(e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }
       
        // Method to manually assign a chore to a ChoreDoer
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

        // Randomly assigns all chosen chores to all doers
        void assignChoresRandomly() {
          try {
            // Check if there are chores to assign
            if (chores.empty()) {
              wxMessageBox(wxT("No chores to assign."), wxT("Info"), wxOK | wxICON_INFORMATION);
              return;
            }

            // Check if there are chore doers available
            if (doers.empty()) {
              wxMessageBox(wxT("No chore doers available."), wxT("Info"), wxOK | wxICON_INFORMATION);
              return;
            }

            // Create a random engine; you could also use default_random_engine
            random_device rd;
            mt19937 g(rd());

            // Shuffle the chores using the random engine
            ranges::shuffle(chores.begin(), chores.end(), g);

            size_t choreIndex = 0;
            // Loop over each chore and try to assign it to a chore doer
            while (choreIndex < chores.size()) {
              for (auto& choreDoer : doers) {
                if (choreIndex < chores.size()) {
                  choreDoer->assignChore(chores[choreIndex++]);
                }
                else {
                  break; // Break if there are no more chores to assign
                }
              }
            }
          }
          catch (const exception& e) {
            wxMessageBox(wxString("Exception thrown in assignChoresRandomly: ") + wxString(e.what()), wxT("Error"), wxOK | wxICON_ERROR);
          }
        }

        wxString displayChoreList() {
          wxString info;
          for (const auto& chore : chores) {
            info += chore->simplePrint(); //Only returns few attributes instead of all attributes
          }
          return info;
        }

        // Returns assigned chores for a chosen chore doer
        wxString displayAssignedChores(const wxString& doerName) {
          auto doer = find_if(doers.begin(), doers.end(), [&doerName](const shared_ptr<ChoreDoer>& d) {
            return d->getName() == doerName;
            });

          if (doer != doers.end()) {
            wxString info = "Chore Doer: " + (*doer)->getName() + " has chores:\n";
            info += (*doer)->assignedChores.returnAllItems();  // Use Container's method
            return info;
          }

          return wxT("Chore Doer not found.");
        }

        // Returns assigned chores for all chore doers
        wxString displayAllChoreAssignments() const {
          wxString info;
          for (const auto& doer : doers) {
            info += "Chore Doer: " + doer->getName() + " has chores:\n";
            info += doer->assignedChores.returnAllItems();  // Use Container's method
            info += "\n";  // Add a newline for separation between doers
          }
          return info;  // Return the complete string
        }

        // Method to add a new chore to the list - not implemented**
        void addChore(const json& choreJson) {
            shared_ptr<Chore> chore;
            if (!choreJson.is_null()) {
                if (choreJson["difficulty"] == "easy") {
                  chore = make_shared<EasyChore>(choreJson);
                }
                else if(choreJson["difficulty"] == "medium") {
                  chore = make_shared<MediumChore>(choreJson);
                }
                else if (choreJson["difficulty"] == "hard") {
                  chore = make_shared<EasyChore>(choreJson);
                }
              
                chores.push_back(chore);
                saveData();  // Save every time a chore is added
            }
            else {
                wxMessageBox("Error adding chore: Invalid JSON", "JSON Error", wxOK | wxICON_ERROR);
            }
        }

        // Output all data to the console or to file
        json toJSON() const {
          json output;
          if (client) {
            output["user_profile"] = client->toJSON();
          }
          
          for (const auto& chore : chores) {
            output["chores"].push_back(chore->toJSON());
          }
          return output;
        }

        // saveData method to overwrite the original data.json file for chores and user data
        void saveData() {
            // Reset our json object because json is getting overwritten
            j.clear();

            // All data changed through class functions modify or set(attribute) or add/create chore  will be saved here
            // Add new data to json object with updated chore list
            j = toJSON();

            // Overwrites original file
            ofstream file(dynamicFile.ToStdString());
            if (file) {
                file << setw(4) << j << endl;
            }
            else {
                wxMessageBox("Error saving file: " + dynamicFile, "File Error", wxOK | wxICON_ERROR);
            }
            file.close();
        }

        // Method to display the client profile
        /* Do not need this function, we will have one user
        bool userExists(const wxString& username) {
            return j["user_profiles"].find(username.ToStdString()) != j["user_profiles"].end();
        }
        */
        
        // Method to create a new user profile
        /*
        void createUser(const wxString& username, const wxString& theme, bool notify) {
            json newUser = {
                {"username", username.ToStdString()},
                {"theme", theme.ToStdString()},
                {"notify", notify}
            };
            j["user_profiles"][username.ToStdString()] = newUser;
            saveData();
        }
        */
        // Method to update the user profile
        void updateUser(const wxString& username, const wxString& theme, bool notify) {
            client->setUsername(username);
            client->setTheme(theme);
            client->setNotify(notify);
            saveData();
            wxMessageBox("Welcome, " + client->getUserName() + "!", "Login Success", wxOK | wxICON_INFORMATION);
        }

        // don't think we need this
        /*
        bool validateUser(wxString w, wxString z) {
            return true; // Placeholder for actual validation logic
        }
        */

        // Method to display the chore list
        //******************************************************************
        //CHANGED DISPLAY CHORES TO WORK WITH WXWIDGETS (void function not allowed)
        Container<Chore> displayChores()
        {
            Container<Chore> choreList;
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

  /* Not sure how to implement this
    // Example integration with a menu in MyFrame
    void MyFrame::OnSortChores(wxCommandEvent& event) {
      switch (event.GetId()) {
      case ID_SORT_DOERS_EARNINGS:
        choreManager.sortAllChoreDoersChoresByEarnings();
        break;
      case ID_SORT_DOERS_DIFFICULTY:
        choreManager.sortAllChoreDoersChoresByDifficulty();
        break;
      case ID_SORT_DOERS_ID:
        choreManager.sortAllChoreDoersChoresByID();
        break;
      default:
        wxMessageBox("Invalid sort option.", "Error", wxOK | wxICON_ERROR);
      }
    }
    */
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
                //We should only have one user (too much confusion)
                /*
                if (!m_choreManager->userExists(username)) {
                    m_choreManager->createUser(username, theme, notify);
                    wxMessageBox("New user created: " + username + ". Welcome to Chore Manager!", "User Created", wxOK | wxICON_INFORMATION);
                }
                */
                
                m_choreManager->updateUser(username, theme, notify);
                wxMessageBox("Welcome back, " + username + "!", "Login Success", wxOK | wxICON_INFORMATION);

                // Close the dialog
                EndModal(wxID_OK);
            }
        };
    };

}

wxIMPLEMENT_APP(ChoreAppNamespace::ChoreApp);  // This macro creates the main() function for the application