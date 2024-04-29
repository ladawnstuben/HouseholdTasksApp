
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
#pragma warning( pop )

using json = nlohmann::json;
using namespace std;

//// Define the starting path to the test data file
const string DATA_FILE_PATH = "TestData\\";

namespace ChoreApp
{

  enum class DIFFICULTY { EASY, MEDIUM, HARD };
  enum class STATUS { NOT_STARTED, IN_PROGRESS, COMPLETED };
  enum class PRIORITY { LOW, MODERATE, HIGH };

  
  class Client {
  private:
    struct Preferences {
      bool notify;
      string theme;  // Using a simple string

      Preferences(const json& j)
        : notify(j.contains("notify") && !j["notify"].is_null() ? j["notify"].get<bool>() : false),
        theme(j.contains("theme") && !j["theme"].is_null() ? j["theme"].get<string>() : "Default") {}  // Use an empty string as the default if theme is not present
    };

    struct UserProfile {
      string username;
      string last_logged_in; // Always set to the current date and time
      Preferences preferences;

      UserProfile(const json& j)
        : username(j.contains("username") && !j["username"].is_null() ? j["username"].get<string>() : "DefaultUser"), // Prompt the user for their username
        last_logged_in(getCurrentDateTime()), // Set to current datetime
        preferences(j.contains("preferences") && !j["preferences"].is_null() ? j["preferences"] : json{}) {}
      // Directly pass the JSON object

      string getCurrentDateTime() {
        time_t now = time(nullptr);  // Get the current time as time_t
        struct tm timeinfo;  // Create a tm struct to hold local time

        // Use localtime_s for safe conversion from time_t to tm.
        // localtime_s returns zero on success.
        // localtime is deprecated in C++14 and later, so it's better to use localtime_s.
        if (localtime_s(&timeinfo, &now) != 0) {
          throw runtime_error("Failed to convert time to local time.");
        }

        stringstream ss;
        ss << put_time(&timeinfo, "%Y-%m-%d %H:%M:%S");  // Format time to a readable string
        return ss.str();
      }
    };
  
    UserProfile userProfile;


  public:
    // Constructor checks if "user_profile" key exists and handles it
    Client(const json& j)
      : userProfile(j.contains("user_profile") ? j["user_profile"] : json{}) {}

    json toJSON() const {
      return json{
          {"username", userProfile.username},
          {"last_logged_in", userProfile.last_logged_in},
          {"preferences", {{"notify", userProfile.preferences.notify}, {"theme", userProfile.preferences.theme}}}
      };
    }

    void modifyProfile() {
      cout << "Modifying Profile..." << endl;
      cout << "Enter new username: ";
      getline(cin, userProfile.username);
      cout << "Enter new last logged in: ";
      getline(cin, userProfile.last_logged_in);
      cout << "Enter new preferences (in JSON format): ";
      string pref_input;
      getline(cin, pref_input);
      userProfile.preferences = json::parse(pref_input);
    }

    string printProfile() const {
      string result;
      result += "Username: " + userProfile.username + "\n";
      result += "Last Logged In: " + (!userProfile.last_logged_in.empty() ? userProfile.last_logged_in : "Never") + "\n";
      result += "Notifications: " + string(userProfile.preferences.notify ? "Enabled" : "Disabled") + "\n";
      result += "Theme: " + (!userProfile.preferences.theme.empty() ? userProfile.preferences.theme : "Default") + "\n";
      return result;
    }

    // before calling, tell user to enter a username with wxWidgets
    // needs to work with wxWidgets
    void setUsername() {
      string result;
      getline(cin, userProfile.username);
      if (userProfile.username.empty()) {
        //cout << "Username cannot be empty. Please try again." << endl;
        setUsername();  // Recursive call if input is empty
      }
    }

    void setNotify()
    {
      // needs to work with wxWidgets
      //cout << "Please enter your notification preference (true/false): ";
      string notify;
      getline(cin, notify);
      if (notify != "true" && notify != "false")
      {
        cout << "Invalid input. Please enter 'true' or 'false'." << endl;
        setNotify();  // Recursive call if input is invalid
      }
      userProfile.preferences.notify = notify == "true";
    }

    void setTheme()
    {
      // needs to work with wxWidgets
      cout << "Please enter your theme (dark/light): ";
      getline(cin, userProfile.preferences.theme);
      if (userProfile.preferences.theme != "dark" && userProfile.preferences.theme != "light")
      {
        cout << "Invalid theme. Please enter 'dark' or 'light'." << endl;
        setTheme();  // Recursive call if input is invalid
      }
    }

    string getUserName() const
    {
      return userProfile.username;
    }

    string getLastLoggedIn() const
    {
      return userProfile.last_logged_in;
    }

    string getNotify() const
    {
      return userProfile.preferences.notify ? "Enabled" : "Disabled";
    }

    string getTheme() const
    {
      return userProfile.preferences.theme.empty() ? "Default" : userProfile.preferences.theme;
    }

    void toggleNotify()
    {
      userProfile.preferences.notify = !userProfile.preferences.notify;
    }

    void toggleTheme()
    {
      if (userProfile.preferences.theme == "dark")
      {
        userProfile.preferences.theme = "light";
      }
      else
      {
        userProfile.preferences.theme = "dark";
      }
    }

    // Friend declaration for the insertion operator
    friend ostream& operator<<(ostream& os, const Client& client);
  };

  ostream& operator<<(ostream& os, const Client& client) {
    os << client.printProfile();  // Assuming printProfile() returns the formatted string
    return os;
  }

  //*********************************************************************************

  class Chore {
  private:
    int id;
    int earnings;

    string name;
    string description;
    string frequency;
    string estimated_time;
    string notes;
    string location;

    vector<string> days;
    vector<string> tools_required;
    vector<string> materials_needed;
    vector<string> tags;

    DIFFICULTY Difficulty;
    STATUS Status;
    PRIORITY Priority;

  public:

    /*Class Definition for a single chore*/
    Chore(const json& j) {
      id = j["id"].is_null() ? -1 : j["id"].get<int>();
      name = j["name"].is_null() ? "" : j["name"].get<string>();
      description = j["description"].is_null() ? "" : j["description"].get<string>();
      frequency = j["frequency"].is_null() ? "" : j["frequency"].get<string>();
      estimated_time = j["estimated_time"].is_null() ? "" : j["estimated_time"].get<string>();
      earnings = j["earnings"].is_null() ? 0 : j["earnings"].get<int>();

      // For vector<string>, need to check each element if the array itself is not null
      days = j["days"].is_null() ? vector<string>() : j["days"].get<vector<string>>();
      location = j["location"].is_null() ? "" : j["location"].get<string>();
      tools_required = j["tools_required"].is_null() ? vector<string>() : j["tools_required"].get<vector<string>>();
      materials_needed = j["materials_needed"].is_null() ? vector<string>() : j["materials_needed"].get<vector<string>>();
      notes = j["notes"].is_null() ? "" : j["notes"].get<string>();
      tags = j["tags"].is_null() ? vector<string>() : j["tags"].get<vector<string>>();
      Difficulty = parseDifficulty(j);
      Priority = parsePriority(j);
      Status = parseStatus(j);
    }

    // Default constructor
    Chore() = default;

    virtual ~Chore() {}


    virtual void startChore() {
      if (Status == STATUS::NOT_STARTED)
      {
        cout << "In Progress: " << name << endl;
        Status = STATUS::IN_PROGRESS;
      }
      else if (Status == STATUS::IN_PROGRESS)
      {
        cout << "Chore already started " << name << endl;
      }
      else if (Status == STATUS::COMPLETED)
      {
        cout << "In Progress: " << name << endl;
        Status = STATUS::IN_PROGRESS;
      }
    }

    virtual void completeChore()
    {
      if (Status == STATUS::NOT_STARTED)
      {
        cout << "Completed: " << name << endl;
        Status = STATUS::COMPLETED;
      }
      else if (Status == STATUS::IN_PROGRESS)
      {
        cout << "Completed: " << name << endl;
        Status = STATUS::COMPLETED;
      }
      else if (Status == STATUS::COMPLETED)
      {
        cout << "Chore already completed: " << name << endl;
      }
    }

    virtual void resetChore() 
    {
      if (Status == STATUS::NOT_STARTED)
      {
        cout << "Chore not started: " << name << endl;
      }
      else if (Status == STATUS::IN_PROGRESS)
      {
        cout << "Resetting: " << name << endl;
        Status = STATUS::NOT_STARTED;
      }
      else if (Status == STATUS::COMPLETED)
      {
        cout << "Resetting: " << name << endl;
        Status = STATUS::NOT_STARTED;
      }
    }


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
          {"difficulty", toStringD(Difficulty)},
          {"priority", toStringP(Priority)},
          {"status", toStringS(Status)}
      };
    }

    virtual string PrettyPrintClassAttributes() const {
      string result = "Chore ID: " + to_string(id) + "\n"
        "Name: " + name + "\n"
        "Description: " + description + "\n"
        "Frequency: " + frequency + "\n"
        "Estimated Time: " + estimated_time + "\n"
        "Earnings: " + to_string(earnings) + "\n"
        "Days: " + formatVector(days) + "\n"
        "Location: " + location + "\n"
        "Tools Required: " + formatVector(tools_required) + "\n"
        "Materials Needed: " + formatVector(materials_needed) + "\n"
        "Notes: " + notes + "\n"
        "Tags: " + formatVector(tags) + "\n"
        "Status: " + toStringS(Status) + "\n"
        "Priority: " + toStringP(Priority) + "\n"
        "Difficulty: " + toStringD(Difficulty);

      return result;
    }


    void modifyChore() {
      cout << "Modifying Chore..." << endl;
      cout << "Enter new chore name: ";
      getline(cin, name);
      cout << "Enter new chore description: ";
      getline(cin, description);
    }


    int getID() const
    {
      return id;
    }

    string getName() const
    {
      return name;
    }

    int getEarnings() const
    {
      return earnings;
    }

    //Handling ENUMS:

    // Getter for MusicType
    STATUS GetStatus() const
    {
      return Status;
    }

    // Getter for MusicType
    PRIORITY GetPriority() const
    {
      return Priority;
    }

    DIFFICULTY getDifficulty() const
    {
      return Difficulty;
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
        this->Difficulty == other.Difficulty &&
        this->Status == other.Status &&
        this->Priority == other.Priority);
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
        return a->getID() < b->getID();
      }
    };

    

  protected:

    static string formatVector(const vector<string>& vec) {
      string result;
      for (const auto& item : vec) {
        if (!result.empty()) result += ", ";
        result += item;
      }
      return result.empty() ? "None" : result;
    }

    // Getter for MusicType
    DIFFICULTY parseDifficulty(const json& j) {
      if (!j.contains("status") || j["status"].is_null()) {
        return DIFFICULTY::EASY;  // Handle null or missing status by returning UNKNOWN
      }
      string dif = j["difficulty"].get<string>();
      if (dif == "easy") return DIFFICULTY::EASY;
      if (dif == "medium") return DIFFICULTY::MEDIUM;
      if (dif == "hard") return DIFFICULTY::HARD;
      return DIFFICULTY::EASY;
    }

    PRIORITY parsePriority(const json& j) {
      if (!j.contains("priority") || j["priority"].is_null()) {
        return PRIORITY::LOW;  // Handle null or missing status by returning UNKNOWN
      }
      string pri = j["priority"].get<string>();
      if (pri == "low") return PRIORITY::LOW;
      if (pri == "moderate") return PRIORITY::MODERATE;
      if (pri == "high") return PRIORITY::HIGH;
      return PRIORITY::LOW; // Default
    }

    STATUS parseStatus(const json& j) {
      if (!j.contains("status") || j["status"].is_null()) {
        return STATUS::NOT_STARTED;  // Handle null or missing status by returning UNKNOWN
      }
      string stat = j["status"].get<string>();
      if (stat == "not_started") return STATUS::NOT_STARTED;
      if (stat == "in_progress") return STATUS::IN_PROGRESS;
      if (stat == "completed") return STATUS::COMPLETED;
      return STATUS::NOT_STARTED;  // Default case if status is not recognized
    }

    // Convert DIFFICULTY enum to string
    string toStringD(DIFFICULTY d) const {
      switch (d) {
      case DIFFICULTY::EASY: return "easy";
      case DIFFICULTY::MEDIUM: return "medium";
      case DIFFICULTY::HARD: return "hard";
      default: return "easy";
      }
    }

    // Convert STATUS enum to string
    string toStringS(STATUS s) const {
      switch (s) {
      case STATUS::NOT_STARTED: return "not started";
      case STATUS::IN_PROGRESS: return "in progress";
      case STATUS::COMPLETED: return "completed";
      default: return "not started";
      }
    }

    // Convert PRIORITY enum to string
    string toStringP(PRIORITY p) const {
      switch (p) {
      case PRIORITY::LOW: return "low";
      case PRIORITY::MODERATE: return "moderate";
      case PRIORITY::HIGH: return "high";
      default: return "low";
      }
    }
  };

  // Implementation of operator<<
  ostream& operator<<(ostream& os, const Chore& chore) {
    os << chore.PrettyPrintClassAttributes();
    return os;
  }

  //*********************************************************************

  /*Class definition of an easy chore*/
  class EasyChore : public Chore {
  private:
    string multitasking_tips;

  public:
    EasyChore(const json& j) : Chore(j)
    {
      multitasking_tips = j["multitasking_tips"];
    }

    void startChore() override {
      cout << "Starting easy chore: " << endl;;
      Chore::startChore();
      cout << "Multitasking Tips: " << multitasking_tips << endl;
    }

    void completeChore() override
    {
      cout << "Completing easy chore: " << endl;
      Chore::completeChore();
      cout << "Earnings from chore: " << getEarnings() << endl;
    }

    void resetChore() override
    {
      cout << "Resetting easy chore: " << endl;
      Chore::resetChore();
    }

    string PrettyPrintClassAttributes() const override {
      return Chore::PrettyPrintClassAttributes() +
        "\nMultitasking Tips: " + multitasking_tips;
    }

    bool operator==(const EasyChore& other) const
    {
      if(Chore::operator==(other))
      {
        return multitasking_tips == other.multitasking_tips;
      }

      return false;
    }

    json toJSON() const override
    {
      json j = Chore::toJSON();
      j += json{ "multitasking_tips",multitasking_tips };
      return j;
    }

    friend ostream& operator<<(ostream& os, const EasyChore& chore);
  };

  //************************************************************************************************

  /*Class definition of an medium chore*/
  class MediumChore : public Chore {
  private:
    vector<string> variations;

  public:

    MediumChore(const json& j) : Chore(j)
    {
      // Directly parse the JSON array to the vector of strings
      variations = j["variations"].is_null() ? vector<string>() : j["variations"].get<vector<string>>();
    }

    void startChore()override {
      cout << "Starting medium chore: " << endl;
      Chore::startChore();  // Optionally call base method if it does something useful
      cout << "Variations available: ";
      for (const auto& variation : variations) {
        cout << variation << (variation != variations.back() ? ", " : "");
      }
      cout << endl;
    }

    void completeChore() override
    {
      cout << "Completing medium chore: " << endl;
      Chore::completeChore();
      cout << "Earnings from chore: " << getEarnings() << endl;
    }

    void resetChore() override
    {
      cout << "Resetting medium chore: " << endl;
      Chore::resetChore();
    }

    string PrettyPrintClassAttributes() const override {
      return Chore::PrettyPrintClassAttributes() +
        "\nVariations: " + formatVector(variations);
    }

    bool operator==(const MediumChore& other) const
    {
      if(Chore::operator==(other))
      {
        return variations == other.variations;
      }

      return false;
    }

    json toJSON() const override
    {
      json j = Chore::toJSON();
      j += json{ "variations", variations };
      return j;
    }

    friend ostream& operator<<(ostream& os, const MediumChore& chore);
  };

  //************************************************************************************************

  /*Class definition of an hard chore*/
  class HardChore : public Chore {
  private:
    struct Subtask {
      string name;
      string estimated_time;
      int earnings;

      Subtask(const json& subtaskJson) :
        name(subtaskJson["name"].is_null() ? "" : subtaskJson["name"].get<string>()),
        estimated_time(subtaskJson["estimated_time"].is_null() ? "" : subtaskJson["estimated_time"].get<string>()),
        earnings(subtaskJson["earnings"].is_null() ? 0 : subtaskJson["earnings"].get<int>()) {}

      // Add equality comparison operator for subtasks
      bool operator==(const Subtask& other) const {
        return name == other.name &&
          estimated_time == other.estimated_time &&
          earnings == other.earnings;
      }
    };


    vector<Subtask> subtasks;

  public:

    HardChore(const json& j) : Chore(j) {
      if (!j["subtasks"].is_null() && j["subtasks"].is_array()) {
        for (const auto& subtaskJson : j["subtasks"]) {
          subtasks.push_back(Subtask(subtaskJson));
        }
      }
    }

    void startChore() override {
      cout << "Starting hard chore: " << endl;
      Chore::startChore();
      for (const auto& subtask : subtasks) {
        cout << "  Subtask: " << subtask.name << ", Time: " << subtask.estimated_time << ", Earnings: $" << to_string(subtask.earnings) << endl;
      }
    }

    void completeChore() override
    {
      cout << "Completing hard chore: " << endl;
      cout << "Earnings from chore: " << getEarnings() << endl;
    }

    void resetChore() override
    {
      cout << "Resetting hard chore: " << endl;
      Chore::resetChore();
    }

    string PrettyPrintClassAttributes() const override {
      string result = Chore::PrettyPrintClassAttributes();
      for (const auto& subtask : subtasks) {
        result += "\nSubtask: " + subtask.name +
          ", Time: " + subtask.estimated_time +
          ", Earnings: $" + to_string(subtask.earnings);
      }
      return result; // Return the result string
    }

    bool operator==(const HardChore& other) const {
      // First, check if the base class attributes are equal
      if (!Chore::operator==(other)) {
        return false;
      }

      // Compare subtasks if base class attributes are equal
      if (this->subtasks.size() != other.subtasks.size()) {
        return false;  // Different number of subtasks means they are not equal
      }

      for (size_t i = 0; i < this->subtasks.size(); ++i) {
        // Compare each subtask element individually
        if (!(this->subtasks[i] == other.subtasks[i])) {
          return false;
        }
      }

      return true;
    }

    json toJSON() const override {
      json j = Chore::toJSON();
      json subtasksJson = json::array();
      for (const auto& subtask : subtasks) {
        json stJson;
        stJson["name"] = subtask.name;
        stJson["estimated_time"] = subtask.estimated_time;
        stJson["earnings"] = subtask.earnings;
        subtasksJson.push_back(stJson);
      }
      j["subtasks"] = subtasksJson;
      return j;
    }
  };

  //*********************************************************************************

  /*Templated Class definition of a Container of shared pointers (Example: vector<shared_prt<Chore>> chores)*/
  template<typename T>
  class Container {
  private:
    vector<shared_ptr<T>> items;

  public:

    // Generic bubble sort using comparator
    template<typename Comparator>
    void sortItems(Comparator comp, bool ascending = true) {
      try
      {
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
      catch (const exception& e)
      {
        cerr << "Exception thrown in sortItems: " << e.what() << endl;
      }
    }

    void moveItemToAnotherContainer(int id, Container<T>& destination) {
      bool found = false;
      for (auto it = items.begin(); it != items.end(); ++it) {
        if ((*it)->getID() == id) {
          destination.items.push_back(*it);  // Add to destination
          cout << "Chore moved successfully: " << (*it)->getName() << endl;
          items.erase(it);  // Remove from source
          found = true;
          break;  // Stop after finding and moving the item
        }
      }

      if (!found) {
        cout << "Chore not found." << endl;
      }
    }

    void deleteItem(int id)
    {
      for (auto it = items.begin(); it != items.end(); ++it)
      {
        if ((*it)->getID() == id)
        {
          items.erase(it);
          break;
        }
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

    void display() const {
      for (const auto& item : items) {
        cout << "Chore: " << item->getName() << " (ID: " << item->getID() << ")" << endl;
      }
    }

    // Checks if the container is empty
    bool empty() const {
      return items.empty();
    }

    // Clears all items from the container
    void clear() {
      items.clear();
    }

    // Method to provide access to the internal items
    const vector<shared_ptr<T>>& item() const {
      return items;
    }

    // Adding begin and end methods to support range-based for loops
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

  //*********************************************************************************

  class ChoreDoer {
  public:

    Container <Chore> assignedChores;

    ChoreDoer(const string& name) : name(name)
    {
      age = 0;
      choreAmount = 0;
      totalEarnings = 0;
      assignedChores = Container<Chore>();
    }

    void assignChore(const shared_ptr<Chore> &chore)  {
      assignedChores.push_back(chore);
      choreAmount++;
    }

    string getName() const
    {
      return name;
    }

    int getTotalEarnings() const
    {
      return totalEarnings;
    }

    int getChoreAmount() const
    {
      return choreAmount;
    }

    string printChoreDoer() const
    {
      string result = "Chore Doer: " + name + "\n";
      result += "Total Earnings: $" + to_string(totalEarnings) + "\n";
      result += "Chore Amount: " + to_string(choreAmount) + "\n";
      return result;
    }

    // Start a chore
    void startChore(int choreId) {
      for (auto& chore : assignedChores) {
        if (chore->getID() == choreId) {
          chore->startChore();
          break;
        }
      }
    }

    void completeChore(int choreId) {
      for (auto& chore : assignedChores) {
        if (chore->getID() == choreId) {
          if (chore->GetStatus() == STATUS::IN_PROGRESS || chore->GetStatus() == STATUS::NOT_STARTED) {
            chore->completeChore(); // Complete the chore
            totalEarnings += chore->getEarnings(); // Update the earnings
            cout << "Chore " << chore->getName() << " completed. Total earnings now: $" << totalEarnings << endl;
            break;
          }
          else {
            cout << "Chore " << chore->getName() << " is already completed. No action taken." << endl;
          }
        }
      }
    }

    // Method to reset a chore
    void resetChore(int choreId) {
      for (auto& chore : assignedChores) {
        if (chore->getID() == choreId) {
          if (chore->GetStatus() == STATUS::COMPLETED || chore->GetStatus() == STATUS::IN_PROGRESS) {
            cout << "Resetting Chore: " << chore->getName() << endl;
            chore->resetChore();
          }
          else {
            cout << "Chore is already in the initial state (Not Started)." << endl;
          }
          return;
        }
      }
      cout << "Chore ID " << choreId << " not found among assigned chores." << endl;
    }

    // overloaded insertion operator
    // Friend declaration for the insertion operator
    friend ostream& operator<<(ostream& os, const ChoreDoer& chDoer);


  private:
    string name;
    int choreAmount;
    // NOT IMPLEMENTED - Assign chore of certain difficulty based on chore doer's age
    int age;
    // NOT IMPLEMENTED - When a chore is completed, the earnings are added to the totalEarnings
    int totalEarnings;
  };

  ostream& operator<<(ostream& os, const ChoreDoer& chDoer) {
    os << chDoer.printChoreDoer();  // Assuming printProfile() returns the formatted string
    return os;
  }

  //*********************************************************************************
  class ChoreManager {

  public:
    ChoreManager(string fileName) : client(json{}) {
      try {
        // Save dyn file name
        dynamicFile = fileName;

        // Seed the random number generator
        srand(static_cast<unsigned int>(time(nullptr)));

        ifstream file(fileName);
        if (file.is_open()) {
          // If JSON object is empty, parse the file into it
          if (j.is_null())
          {
            j = json::parse(file);
          }
          file.close();
        }
        else
        {
          cerr << "Failed to open file: " << fileName << endl;
        }
        // Initialize client with the user profile part of the JSON
        if (j.contains("user_profile")) {
          if (this->client.getUserName() == "DefaultUser")
          {
            this->client.setUsername();
          }
        }
        choreCount = 0;
        loadChores();
      }
      catch (const json::exception& e) {
        cerr << "JSON Error: " << e.what() << endl;
        throw;  // Rethrow to handle the error in main or higher up in the call stack
      }
      catch (const ifstream::failure& e) {
        cerr << "File Error: " << e.what() << endl;
        throw;
      }
    }

    ~ChoreManager()
    {
      clearAll();
    }

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

            Chores.push_back(chore); // Adding to the container
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

    void modifyData() {
      cout << "1: Modify User Profile\n2: Modify Chores\nChoose option: ";
      int choice;
      cin >> choice;
      cin.ignore(); // clear buffer after reading number

      switch (choice) {
      case 1:
        client.modifyProfile();
        break;
      case 2:
        cout << "Enter chore index to modify: ";
        int index;
        cin >> index;
        cin.ignore();
        if (index >= 0 && index < Chores.size()) {
          Chores[index]->modifyChore();
        }
        else {
          cout << "Invalid index!" << endl;
        }
        break;
      default:
        cout << "Invalid choice!" << endl;
      }
      saveData();
    }

    // Method to output all data including chore assignments and user profile to a file
    void outputChoreAssignmentsToFile(const string& outputPath) {
      json output;

      // Include client's user profile data
      output["user_profile"] = client.toJSON();

      // Include chore assignments for each chore doer
      output["chore_doers"] = json::array();
      for (const auto& doer : ChoreDoers) {
        json doerJson;
        doerJson["name"] = doer.getName();
        doerJson["chores"] = json::array();
        for (const auto& chore : doer.assignedChores) {
          doerJson["chores"].push_back(chore->toJSON());
        }
        output["chore_doers"].push_back(doerJson);
      }

      // Add leftover chores
      output["leftover_chores"] = json::array();
      for (const auto& chore : LeftoverChores) {
        output["leftover_chores"].push_back(chore->toJSON());
      }

      // Write to file with pretty formatting
      ofstream outFile(outputPath);
      if (!outFile.is_open()) {
        throw runtime_error("Could not open file to write chore assignments.");
      }
      outFile << setw(4) << output; // Pretty print with indent of 4 spaces
      outFile.close();
    }

    // Rewrite the chores file with the current chores
    void saveData() {
      j["user_profile"] = client.toJSON();
      for (const auto& chore : Chores) {
        j["chores"].push_back(chore->toJSON());
      }

      ofstream file(dynamicFile);
      file << setw(4) << j << endl; // Pretty print with 4 spaces indent
      file.close();
    }

    // Output all data to the console or to file
    json toJSON() const {
      json output;
      output["user_profile"] = client.toJSON();
      for (const auto& chore : Chores) {
        output["chores"].push_back(chore->toJSON());
      }
      return output;
    }

    void displayChores() const {
      for (const auto& chore : Chores) {
        cout << "Chore: " << chore->getName() << " (ID: " << chore->getID() << ")" << endl;
      }
    }

    void displayChoreAssignments() const {
      for (const auto& doer : ChoreDoers) {
        cout << "Chore Doer: " << doer.getName() << " has chores: ";
        for (const auto& chore : doer.assignedChores) {
          cout << chore->getName() << " (ID: " << chore->getID() << "), ";
        }
        cout << endl;
      }
      if (!LeftoverChores.empty()) {
        cout << "Leftover Chores: ";
        for (const auto& chore : LeftoverChores) {
          cout << chore->getName() << " (ID: " << chore->getID() << "), ";
        }
        cout << endl;
      }
    }

    void deleteChoreFromAvailable(int choreId) {
      Chores.deleteItem(choreId);
    }

    void deleteLeftoverChore(int choreId)
    {
      LeftoverChores.deleteItem(choreId);
    }

    void deleteChoreFromAnyDoer(int choreId) {
      for (auto& doer : ChoreDoers) {
        doer.assignedChores.deleteItem(choreId);
      }
    }

    void addChoreDoer(const string& name) {
      ChoreDoers.push_back(ChoreDoer(name));
    }

    void deleteChoreDoer(const string& name)
    {
      try
      {
        for (auto it = ChoreDoers.begin(); it != ChoreDoers.end(); ++it)
        {
          if (it->getName() == name)
          {
            ChoreDoers.erase(it);
            break;
          }
        }
      }
      catch (const exception& e)
      {
        cerr << "Exception thrown in removeChoreDoer: " << e.what() << endl;
      }
    }

    void addChore(const json& choreJson) {
      shared_ptr<Chore> chore;

      if (choreJson.is_null())
      {
        cerr << "Invalid chore JSON." << endl;
        return;
      }
      if(choreJson.contains("difficulty") && choreJson["difficulty"].is_string())
      {
        string difficulty = choreJson["difficulty"].get<string>();
        
        if (difficulty == "easy")
        {
          chore = make_shared<EasyChore>(choreJson);
        }
        else if (difficulty == "medium")
        {
          chore = make_shared<MediumChore>(choreJson);
        }
        else if (difficulty == "hard")
        {
          chore = make_shared<HardChore>(choreJson);
        }
      }
      Chores.push_back(chore);
      saveData();  // Add new chore and save data
    }


    

    void moveChoreToLeftover(int choreId) {
      for (auto& doer : ChoreDoers) {
        doer.assignedChores.moveItemToAnotherContainer(choreId, LeftoverChores);
      }
    }

    void moveChoreFromLeftoverToDoer(const string& toDoer, int choreId) {
      for (auto& doer : ChoreDoers) {
        if (doer.getName() == toDoer) {
          LeftoverChores.moveItemToAnotherContainer(choreId, doer.assignedChores);
          break;
        }
        else
        {
          cout << "Chore Doer not found." << endl;
        }
      }
    }

    void moveChoreBetweenDoers(const string& fromDoer, const string& toDoer, int choreId) {
      bool found = false;
      for (auto& doer : ChoreDoers) {
        if (doer.getName() == fromDoer) {
          for (auto& targetDoer : ChoreDoers) {
            if (targetDoer.getName() == toDoer) {
              doer.assignedChores.moveItemToAnotherContainer(choreId, targetDoer.assignedChores);
              found = true;
              break;
            }
          }
        }
      }
      if (!found) {
        cout << "Chore or Chore Doer not found." << endl;
      }
    }

    void assignChoresRandomly() {
      try
      {
        size_t choreIndex = 0;
        while (choreIndex < Chores.size()) {
          for (auto& choreDoer : ChoreDoers) {
            if (choreIndex < Chores.size()) {
              choreDoer.assignChore(Chores[choreIndex++]);
            }
            else {
              // When there are no more chore doers left to assign evenly, add leftovers
              break;
            }
          }
        }

        // Handle leftover chores
        while (choreIndex < Chores.size()) {
          LeftoverChores.push_back(Chores[choreIndex++]);
        }
      }
      catch (const exception& e)
      {
        cerr << "Exception thrown in assignChoresRandomly: " << e.what() << endl;
      }
    }

    void createMinimalChore()
    {
      
    }

    void createFullChore() {
      // Basic chore details
      string name, description, frequency, estimated_time, difficulty, days, location,
      tools_required, materials_needed, priority, notes, status, tags, multitasking_tips,
      earningsInput, input;
      double earnings;

      // Variables for handling unique attributes (Easy, Medium, Hard)
      json choreJson;
      char choice;
      choreJson["id"] = choreCount++;

      cout << "Enter chore name: ";
      getline(cin, name);
      choreJson["name"] = name.empty() ? "" : name;

      cout << "Enter chore description: ";
      getline(cin, description);
      choreJson["description"] = description.empty() ? "" : description;

      cout << "Enter chore frequency (e.g., daily, weekly): ";
      getline(cin, frequency);
      choreJson["frequency"] = frequency.empty() ? "" : frequency;

      cout << "Enter estimated time for chore completion: ";
      getline(cin, estimated_time);
      choreJson["estimated_time"] = estimated_time.empty() ? "" : estimated_time;

      // Handle earnings input with robust validation
      while (true) {
        cout << "Enter earnings for chore (integer value): ";
        getline(cin, earningsInput);
        try {
          earnings = stoi(earningsInput);
          choreJson["earnings"] = earnings;
          break;  // Break out of the loop if stoi succeeds
        }
        catch (const exception& e) {  // Catching all exceptions
          cout << "Invalid input for earnings. Please enter a valid integer." << e.what() << endl;
        }
      }

      cout << "Enter days when the chore should be performed (comma separated): ";
      getline(cin, days);
      choreJson["days"] = days.empty() ? json::array() : json::parse("[" + days + "]");

      cout << "Enter chore location: ";
      getline(cin, location);
      choreJson["location"] = location.empty() ? json::array() : json::parse("[" + location + "]");


      cout << "Enter tools required (comma separated): ";
      getline(cin, tools_required);
      choreJson["tools_required"] = tools_required.empty() ? json::array() : json::parse("[" + tools_required + "]");

      cout << "Enter materials needed (comma separated): ";
      getline(cin, materials_needed);
      choreJson["materials_needed"] = materials_needed.empty() ? json::array() : json::parse("[" + materials_needed + "]");

      cout << "Enter any additional notes: ";
      getline(cin, notes);
      choreJson["notes"] = notes.empty() ? "" : notes;

      cout << "Enter tags for the chore (comma separated): ";
      getline(cin, tags);
      choreJson["tags"] = tags.empty() ? json::array() : json::parse("[" + tags + "]");

      cout << "Enter difficulty (easy, medium, hard): ";
      getline(cin, difficulty);
      while (difficulty != "easy" && difficulty != "medium" && difficulty != "hard" && !difficulty.empty()) {
        cout << "Invalid difficulty. Please enter 'easy', 'medium', or 'hard': ";
        getline(cin, difficulty);
      }
      choreJson["difficulty"] = difficulty.empty() ? "" : difficulty;

      cout << "Enter priority (low, moderate, high): ";
      getline(cin, priority);
      while (priority != "low" && priority != "moderate" && priority != "high" && !priority.empty()) {
        cout << "Invalid priority. Please enter 'low', 'moderate', or 'high': ";
        getline(cin, priority);
      }
      choreJson["priority"] = priority.empty() ? "" : priority;

      cout << "Enter status (not_started, in_progress, completed): ";
      getline(cin, status);
      while (status != "not_started" && status != "in_progress" && status != "completed" && !status.empty()) {
        cout << "Invalid status. Please enter 'not_started', 'in_progress', or 'completed': ";
        getline(cin, status);
      }
      choreJson["status"] = status.empty() ? "" : status;

      // Unique attributes based on difficulty
      if (difficulty == "easy") {
        cout << "Enter multitasking tips for easy chore: ";
        getline(cin, multitasking_tips);
        choreJson["multitasking_tips"] = multitasking_tips.empty() ? "" : multitasking_tips;
      }
      else if (difficulty == "medium") {
        
        cout << "Are there any variations? Enter 'y' for yes or 'n' for no: ";
        cin >> choice;

        if (choice == 'y')
        {
          cout << "Enter variations for medium chore (comma separated): ";
          cin.ignore();
          getline(cin, input);
          choreJson["variations"] = input.empty() ? json::array() : json::parse("[" + input + "]");
        }
      }
      else if (difficulty == "hard") {
        json subtask;
        string subtaskName;
        string subtaskTime;
        int subtaskEarnings;

        cout << "Are there any subtasks? Enter 'y' for yes or 'n' for no: ";
        cin >> choice;

        if (choice == 'y')
        {
          cout << "Enter a name for one subtask: ";
          getline(cin, subtaskName);

          cout << "Enter estimated time for the subtask: ";
          getline(cin, subtaskTime);

          // Handle earnings input with robust validation
          while (true) {
            cout << "Enter earnings for chore (integer value) or whole dollar amount: ";
            
            try {
              cin >> subtaskEarnings;
              choreJson["earnings"] = subtaskEarnings; // If not int -> catch exception -> repeat loop
              break;  // Break out of the loop if stoi succeeds
            }
            catch (const exception& e) {  // Catching all exceptions
              cout << "Invalid input for earnings. Please enter a valid integer." << e.what() << endl;
            }
          }

          subtask["name"] = subtaskName.empty() ? "" : subtaskName;
          subtask["estimated_time"] = subtaskTime.empty() ? "" : subtaskTime;
          subtask["earnings"] = to_string(subtaskEarnings).empty() ? 0 : subtaskEarnings;
          choreJson["subtasks"] = json::array({ subtask });
        }
        else
        {
          cout << "No subtasks added." << endl;
          choreJson["subtasks"] = json::array(); // should be empty
        }
      }

      // Add the chore to the system
      addChore(choreJson);
      cout << "Chore added successfully!" << endl;
    }

    template<typename Comparator>
    void sortChores(Comparator comp, bool ascending = true) {
      try
      {
        Chores.sortItems(comp, ascending);
      }
      catch (const exception& e)
      {
        cerr << "Exception caught in sortChores: " << e.what() << endl;
      }
    }

  private:
    json j;
    int choreCount;
    Container<Chore> Chores;
    vector<ChoreDoer> ChoreDoers;
    Container<Chore> LeftoverChores;
    Client client;  // Client is now part of ChoreManager
    string dynamicFile;

    void clearAll() {
      try
      {
        // Clear original Container of chores
        clearChores();
        // Clear leftover Container of chores
        clearLeftoverChores();
        // Clear assigned Container of chores for each chore doer
        clearAllAssignedChores();
        // Clear chore doers
        clearAllChoreDoers();
      }
      catch (const exception& e)
      {
        cerr << "Exception caught in clearChores: " << e.what() << endl;
      }
    }

    void clearLeftoverChores()
    {
      try
      {
        if(!LeftoverChores.empty())
        {
          LeftoverChores.clear();
        }
      }
      catch (const exception& e)
      {
        cerr << "Exception caught in clearLeftoverChores: " << e.what() << endl;
      }
    }

    void clearChores()
    {
      try
      {
        if (!Chores.empty())
        {
          Chores.clear();
        }
      }
      catch (const exception& e)
      {
        cerr << "Exception caught in DeleteChores: " << e.what() << endl;
      }
    }

    void clearChoreDoers()
    {
      try
      {
        ChoreDoers.clear();
      }
      catch (const exception& e)
      {
        cerr << "Exception caught in clearChoreDoers: " << e.what() << endl;
      }
    }

    void clearAllAssignedChores()
    {
      if (!ChoreDoers.empty())
      {
        // Clear assigned Container of chores for each chore doer
        for (auto& doer : ChoreDoers)
        {
          if (!doer.assignedChores.empty())
          {
            doer.assignedChores.clear();
          }
        }
      }
    }

    void clearAllChoreDoers()
    {
      // Clear chore doers
      if (!ChoreDoers.empty())
      {
        ChoreDoers.clear();
      }
    }
  };
}

int main() {
  try {
    string testFile(DATA_FILE_PATH + "chores3.json");
    ChoreApp::ChoreManager manager(testFile);
    manager.displayChores();
    manager.addChoreDoer("John");
    manager.addChoreDoer("Jane");
    manager.addChoreDoer("Alice");
    manager.assignChoresRandomly();
    manager.displayChoreAssignments();

  }
  catch (const exception& e) {
    cout << "Exception caught in main: " << e.what() << endl;
  }
  return 0;
}


/*
// Function to cycle through DIFFICULTY values
DIFFICULTY increaseDifficulty(DIFFICULTY& d) {
  switch (d) {
  case DIFFICULTY::EASY:
    d = DIFFICULTY::MEDIUM;
    break;
  case DIFFICULTY::MEDIUM:
    d = DIFFICULTY::HARD;
    break;
  case DIFFICULTY::HARD:
    d = DIFFICULTY::EASY;
    break;
  default:
    d = DIFFICULTY::EASY;
  }
  return d;
}

DIFFICULTY decreaseDifficulty(DIFFICULTY& d)
{
  switch (d)
  {
  case DIFFICULTY::EASY:
    d = DIFFICULTY::HARD;
    break;
  case DIFFICULTY::MEDIUM:
    d = DIFFICULTY::EASY;
    break;
  case DIFFICULTY::HARD:
    d = DIFFICULTY::MEDIUM;
    break;
  default:
    d = DIFFICULTY::EASY;
  }
  return d;
}

// Function to cycle through STATUS values
STATUS increaseStatus(STATUS& s) const {
  switch (s) {
  case STATUS::NOT_STARTED:
    s = STATUS::IN_PROGRESS;
    break;
  case STATUS::IN_PROGRESS:
    s = STATUS::COMPLETED;
    break;
  case STATUS::COMPLETED:
    s = STATUS::NOT_STARTED;
    break;
  default:
    s = STATUS::COMPLETED;
  }
  return s;
}

STATUS decreaseStatus(STATUS& s) const
{
  switch (s)
  {
  case STATUS::NOT_STARTED:
    s = STATUS::COMPLETED;
    break;
  case STATUS::IN_PROGRESS:
    s = STATUS::NOT_STARTED;
    break;
  case STATUS::COMPLETED:
    s = STATUS::IN_PROGRESS;
    break;
  default:
    s = STATUS::NOT_STARTED;
  }
  return s;
}

// Function to cycle through PRIORITY values
PRIORITY increasePriority(PRIORITY& p) {
  switch (p) {
  case PRIORITY::LOW:
    p = PRIORITY::MODERATE;
    break;
  case PRIORITY::MODERATE:
    p = PRIORITY::HIGH;
    break;
  case PRIORITY::HIGH:
    p = PRIORITY::LOW;
    break;
  default:
    p = PRIORITY::HIGH;
  }
  return p;
}

PRIORITY decreasePriority(PRIORITY& p)
{
  switch (p)
  {
  case PRIORITY::LOW:
    p = PRIORITY::HIGH;
    break;
  case PRIORITY::MODERATE:
    p = PRIORITY::LOW;
    break;
  case PRIORITY::HIGH:
    p = PRIORITY::MODERATE;
    break;
  default:
    p = PRIORITY::LOW;
  }
  return p;
}
*/