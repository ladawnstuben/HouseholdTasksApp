
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

  // Definition of the Chore class
  class Chore {
  private:
    // Member variables to store chore details
    int id;  // Unique identifier for the chore
    int earnings;  // Monetary reward for completing the chore

    // Descriptive attributes of the chore
    string name;  // Name of the chore
    string description;  // Description of what the chore entails
    string frequency;  // How often the chore needs to be done
    string estimated_time;  // Estimated time to complete the chore
    string notes;  // Additional notes about the chore
    string location;  // Location where the chore needs to be done

    // Lists that describe when and how the chore is to be done
    vector<string> days;  // Days of the week the chore is active
    vector<string> tools_required;  // Tools required to complete the chore
    vector<string> materials_needed;  // Materials needed to complete the chore
    vector<string> tags;  // Tags for categorizing or searching for the chore

    // Enumerated types for managing chore metadata
    DIFFICULTY Difficulty;  // Difficulty level of the chore
    STATUS Status;  // Current status of the chore (e.g., not started, in progress)
    PRIORITY Priority;  // Priority level of the chore

  public:
    // Constructor that initializes a Chore object from a JSON object
    Chore(const json& j) {
      // Parsing JSON to initialize member variables
      id = j["id"].is_null() ? -1 : j["id"].get<int>();
      name = j["name"].is_null() ? "" : j["name"].get<string>();
      description = j["description"].is_null() ? "" : j["description"].get<string>();
      frequency = j["frequency"].is_null() ? "" : j["frequency"].get<string>();
      estimated_time = j["estimated_time"].is_null() ? "" : j["estimated_time"].get<string>();
      earnings = j["earnings"].is_null() ? 0 : j["earnings"].get<int>();

      // Initializing vectors from JSON arrays, defaulting to empty if not provided
      days = j["days"].is_null() ? vector<string>() : j["days"].get<vector<string>>();
      location = j["location"].is_null() ? "" : j["location"].get<string>();
      tools_required = j["tools_required"].is_null() ? vector<string>() : j["tools_required"].get<vector<string>>();
      materials_needed = j["materials_needed"].is_null() ? vector<string>() : j["materials_needed"].get<vector<string>>();
      notes = j["notes"].is_null() ? "" : j["notes"].get<string>();
      tags = j["tags"].is_null() ? vector<string>() : j["tags"].get<vector<string>>();

      // Initialize enum values using helper functions
      Difficulty = parseDifficulty(j);
      Priority = parsePriority(j);
      Status = parseStatus(j);
    }

    // Default constructor
    Chore() = default;

    // Virtual destructor
    virtual ~Chore() {}

    // Method to start a chore, modifying its status based on current state
    virtual void startChore() {
      if (Status == STATUS::NOT_STARTED) {
        cout << "In Progress: " << name << endl;
        Status = STATUS::IN_PROGRESS;
      }
      else if (Status == STATUS::IN_PROGRESS) {
        cout << "Chore already started " << name << endl;
      }
      else if (Status == STATUS::COMPLETED) {
        cout << "In Progress: " << name << endl;
        Status = STATUS::IN_PROGRESS;
      }
    }

    // Method to mark a chore as completed, modifying its status based on current state
    virtual void completeChore() {
      if (Status == STATUS::NOT_STARTED || Status == STATUS::IN_PROGRESS) {
        cout << "Completed: " << name << endl;
        Status = STATUS::COMPLETED;
      }
      else if (Status == STATUS::COMPLETED) {
        cout << "Chore already completed: " << name << endl;
      }
    }

    // Method to reset a chore to not started, regardless of its current state
    virtual void resetChore() {
      if (Status == STATUS::NOT_STARTED) {
        cout << "Chore not started: " << name << endl;
      }
      else if (Status == STATUS::IN_PROGRESS || Status == STATUS::COMPLETED) {
        cout << "Resetting: " << name << endl;
        Status = STATUS::NOT_STARTED;
      }
    }

    // Serialize the chore object to JSON
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

    // Method to return a formatted string containing all chore attributes for display
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

    // Method to modify chore name and description interactively
    virtual void modifyChore() {
      cout << "Modifying Chore..." << endl;
      cout << "Enter new chore name: ";
      getline(cin, name);
      cout << "Enter new chore description: ";
      getline(cin, description);
    }

    // Accessor methods
    int getID() const { return id; }
    string getName() const { return name; }
    int getEarnings() const { return earnings; }
    STATUS GetStatus() const { return Status; }
    PRIORITY GetPriority() const { return Priority; }
    DIFFICULTY getDifficulty() const { return Difficulty; }

    // Friend declaration for operator<<
    friend ostream& operator<<(ostream& os, const Chore& chore);

    // Comparison operator for checking equality between two chores
    virtual bool operator==(const Chore& other) const {
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

    // Comparison structures for sorting chores based on specific attributes
    struct CompareEarnings {
      bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
        return a->getEarnings() < b->getEarnings();
      }
    };

    struct CompareDifficulty {
      bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
        return a->getDifficulty() < b->getDifficulty(); // Ensure Difficulty can be compared
      }
    };

    struct CompareID {
      bool operator()(const shared_ptr<Chore>& a, const shared_ptr<Chore>& b) const {
        return a->getID() < b->getID();
      }
    };

  protected:
    // Helper method to format a vector of strings for display
    static string formatVector(const vector<string>& vec) {
      string result;
      for (const auto& item : vec) {
        if (!result.empty()) result += ", ";
        result += item;
      }
      return result.empty() ? "None" : result;
    }

    // Helper methods to parse enumeration values from JSON
    DIFFICULTY parseDifficulty(const json& j) {
      if (!j.contains("difficulty") || j["difficulty"].is_null()) {
        return DIFFICULTY::EASY;  // Default to EASY if not specified
      }
      string dif = j["difficulty"].get<string>();
      if (dif == "easy") return DIFFICULTY::EASY;
      if (dif == "medium") return DIFFICULTY::MEDIUM;
      if (dif == "hard") return DIFFICULTY::HARD;
      return DIFFICULTY::EASY;  // Default case
    }

    PRIORITY parsePriority(const json& j) {
      if (!j.contains("priority") || j["priority"].is_null()) {
        return PRIORITY::LOW;  // Default to LOW if not specified
      }
      string pri = j["priority"].get<string>();
      if (pri == "low") return PRIORITY::LOW;
      if (pri == "moderate") return PRIORITY::MODERATE;
      if (pri == "high") return PRIORITY::HIGH;
      return PRIORITY::LOW;  // Default case
    }

    STATUS parseStatus(const json& j) {
      if (!j.contains("status") || j["status"].is_null()) {
        return STATUS::NOT_STARTED;  // Default to NOT_STARTED if not specified
      }
      string stat = j["status"].get<string>();
      if (stat == "not started") return STATUS::NOT_STARTED;
      if (stat == "in progress") return STATUS::IN_PROGRESS;
      if (stat == "completed") return STATUS::COMPLETED;
      return STATUS::NOT_STARTED;  // Default case
    }

    // Methods to convert ENUM values to strings for output
    string toStringD(DIFFICULTY d) const {
      switch (d) {
      case DIFFICULTY::EASY: return "easy";
      case DIFFICULTY::MEDIUM: return "medium";
      case DIFFICULTY::HARD: return "hard";
      default: return "easy";  // Default to "easy" if unknown
      }
    }

    string toStringS(STATUS s) const {
      switch (s) {
      case STATUS::NOT_STARTED: return "not started";
      case STATUS::IN_PROGRESS: return "in progress";
      case STATUS::COMPLETED: return "completed";
      default: return "not started";  // Default to "not started" if unknown
      }
    }

    string toStringP(PRIORITY p) const {
      switch (p) {
      case PRIORITY::LOW: return "low";
      case PRIORITY::MODERATE: return "moderate";
      case PRIORITY::HIGH: return "high";
      default: return "low";  // Default to "low" if unknown
      }
    }
  };

  // Implementation of the operator<< for Chore class
  ostream& operator<<(ostream& os, const Chore& chore) {
    os << chore.PrettyPrintClassAttributes();
    return os;
  }

  //*********************************************************************

  /*Class definition of an easy chore, derived from Chore*/
  class EasyChore : public Chore {
  private:
    string multitasking_tips;  // Tips for multitasking while performing the chore

  public:
    // Constructor that initializes an EasyChore from a JSON object
    EasyChore(const json& j) : Chore(j) {
      multitasking_tips = j["multitasking_tips"];  // Extract multitasking tips from JSON
    }

    // Override the startChore method to include multitasking tips
    void startChore() override {
      cout << "Starting easy chore: " << endl;
      Chore::startChore();  // Call base class implementation
      cout << "Multitasking Tips: " << multitasking_tips << endl;  // Output multitasking tips
    }

    // Override the completeChore method to add behavior for easy chores
    void completeChore() override {
      cout << "Completing easy chore: " << endl;
      Chore::completeChore();  // Call base class implementation
      cout << "Earnings from chore: " << getEarnings() << endl;  // Output earnings from the chore
    }

    // Override the resetChore method to add behavior specific to easy chores
    void resetChore() override {
      cout << "Resetting easy chore: " << endl;
      Chore::resetChore();  // Call base class implementation
    }

    // Provide a string representation of the class's attributes
    string PrettyPrintClassAttributes() const override {
      return Chore::PrettyPrintClassAttributes() +
        "\nMultitasking Tips: " + multitasking_tips;  // Append multitasking tips to output
    }

    // Equality comparison to check if two EasyChore objects are the same
    bool operator==(const EasyChore& other) const {
      if (Chore::operator==(other)) {  // First compare base class attributes
        return multitasking_tips == other.multitasking_tips;  // Then compare multitasking tips
      }
      return false;
    }

    // Convert the object's data to JSON, adding specific attributes
    json toJSON() const override {
      json j = Chore::toJSON();  // Start with base class JSON
      j["multitasking_tips"] = multitasking_tips;  // Add multitasking tips
      return j;
    }
  };

  //************************************************************************************************

  /*Class definition of a medium chore, derived from Chore*/
  class MediumChore : public Chore {
  private:
    vector<string> variations;  // List of variations of the medium chore

  public:
    // Constructor that initializes a MediumChore from a JSON object
    MediumChore(const json& j) : Chore(j) {
      variations = j["variations"].is_null() ? vector<string>() : j["variations"].get<vector<string>>();  // Parse variations from JSON
    }

    // Override the startChore method to display available variations
    void startChore() override {
      cout << "Starting medium chore: " << endl;
      Chore::startChore();  // Call base class implementation
      cout << "Variations available: ";
      for (const auto& variation : variations) {
        cout << variation << (variation != variations.back() ? ", " : "");  // Output each variation
      }
      cout << endl;
    }

    // Override the completeChore method to add behavior for medium chores
    void completeChore() override {
      cout << "Completing medium chore: " << endl;
      Chore::completeChore();  // Call base class implementation
      cout << "Earnings from chore: " << getEarnings() << endl;  // Output earnings from the chore
    }

    // Override the resetChore method for medium chores
    void resetChore() override {
      cout << "Resetting medium chore: " << endl;
      Chore::resetChore();  // Call base class implementation
    }

    // Provide a string representation of the class's attributes including variations
    string PrettyPrintClassAttributes() const override {
      return Chore::PrettyPrintClassAttributes() +
        "\nVariations: " + formatVector(variations);  // Append variations to output
    }

    // Equality comparison to check if two MediumChore objects are the same
    bool operator==(const MediumChore& other) const {
      if (Chore::operator==(other)) {  // First compare base class attributes
        return variations == other.variations;  // Then compare variations
      }
      return false;
    }

    // Convert the object's data to JSON, including variations
    json toJSON() const override {
      json j = Chore::toJSON();  // Start with base class JSON
      j["variations"] = variations;  // Add variations
      return j;
    }
  };

  //************************************************************************************************

  /*Class definition of a hard chore, derived from Chore*/
  class HardChore : public Chore {
  private:
    // Structure to represent subtasks within a hard chore
    struct Subtask {
      string name;  // Name of the subtask
      string estimated_time;  // Estimated time to complete the subtask
      int earnings;  // Potential earnings from completing the subtask

      // Constructor that initializes a Subtask from a JSON object
      Subtask(const json& subtaskJson) :
        name(subtaskJson["name"].is_null() ? "" : subtaskJson["name"].get<string>()),
        estimated_time(subtaskJson["estimated_time"].is_null() ? "" : subtaskJson["estimated_time"].get<string>()),
        earnings(subtaskJson["earnings"].is_null() ? 0 : subtaskJson["earnings"].get<int>()) {}

      // Equality comparison operator for Subtask
      bool operator==(const Subtask& other) const {
        return name == other.name &&
          estimated_time == other.estimated_time &&
          earnings == other.earnings;
      }
    };

    vector<Subtask> subtasks;  // List of subtasks for the hard chore

  public:
    // Constructor that initializes a HardChore from a JSON object
    HardChore(const json& j) : Chore(j) {
      if (!j["subtasks"].is_null() && j["subtasks"].is_array()) {
        for (const auto& subtaskJson : j["subtasks"]) {
          subtasks.push_back(Subtask(subtaskJson));  // Initialize subtasks from JSON array
        }
      }
    }

    // Override the startChore method to display subtask details
    void startChore() override {
      cout << "Starting hard chore: " << endl;
      Chore::startChore();  // Call base class implementation
      for (const auto& subtask : subtasks) {
        cout << "  Subtask: " << subtask.name << ", Time: " << subtask.estimated_time << ", Earnings: $" << to_string(subtask.earnings) << endl;
      }
    }

    // Override the completeChore method to add behavior for hard chores
    void completeChore() override {
      cout << "Completing hard chore: " << endl;
      cout << "Earnings from chore: " << getEarnings() << endl;  // Output earnings from the chore
    }

    // Override the resetChore method for hard chores
    void resetChore() override {
      cout << "Resetting hard chore: " << endl;
      Chore::resetChore();  // Call base class implementation
    }

    // Provide a string representation of the class's attributes including subtasks
    string PrettyPrintClassAttributes() const override {
      string result = Chore::PrettyPrintClassAttributes();  // Start with base class attributes
      for (const auto& subtask : subtasks) {
        result += "\nSubtask: " + subtask.name +
          ", Time: " + subtask.estimated_time +
          ", Earnings: $" + to_string(subtask.earnings);  // Append each subtask details
      }
      return result;  // Return the formatted string
    }

    // Equality comparison to check if two HardChore objects are the same
    bool operator==(const HardChore& other) const {
      if (!Chore::operator==(other)) {  // First compare base class attributes
        return false;
      }

      if (this->subtasks.size() != other.subtasks.size()) {  // Ensure both have the same number of subtasks
        return false;
      }

      for (size_t i = 0; i < this->subtasks.size(); ++i) {
        if (!(this->subtasks[i] == other.subtasks[i])) {  // Compare each subtask individually
          return false;
        }
      }

      return true;  // Return true if all checks pass
    }

    // Convert the object's data to JSON, including subtasks
    json toJSON() const override {
      json j = Chore::toJSON();  // Start with base class JSON
      json subtasksJson = json::array();
      for (const auto& subtask : subtasks) {
        json stJson;
        stJson["name"] = subtask.name;
        stJson["estimated_time"] = subtask.estimated_time;
        stJson["earnings"] = subtask.earnings;
        subtasksJson.push_back(stJson);  // Add each subtask to the JSON array
      }
      j["subtasks"] = subtasksJson;  // Add subtasks array to the chore JSON
      return j;
    }
  };

  //*********************************************************************************

  /*Templated Class definition of a Container of shared pointers (Example: vector<shared_ptr<Chore>> chores)*/
  template<typename T>
  class Container {
  private:
    vector<shared_ptr<T>> items;  // A vector to store shared pointers to objects of type T

  public:
    // Generic bubble sort using comparator
    template<typename Comparator>
    void sortItems(Comparator comp, bool ascending = true) {
      try {
        bool swapped;  // Flag to keep track of whether any items were swapped during a pass
        do {
          swapped = false;  // Reset swapped flag for each pass
          for (size_t i = 1; i < items.size(); i++) {  // Loop through items
            // Perform comparison and possibly swap items
            if ((ascending && comp(items[i - 1], items[i])) || (!ascending && comp(items[i], items[i - 1]))) {
              swap(items[i - 1], items[i]);  // Swap the items if needed
              swapped = true;  // Set swapped flag if a swap has occurred
            }
          }
        } while (swapped);  // Continue sorting until no swaps are made in a pass
      }
      catch (const exception& e) {
        cerr << "Exception thrown in sortItems: " << e.what() << endl;  // Output any exceptions to standard error
      }
    }

    // Move an item from this container to another container based on its ID
    void moveItemToAnotherContainer(int id, Container<T>& destination) {
      bool found = false;  // Flag to check if item was found
      for (auto it = items.begin(); it != items.end(); ++it) {  // Iterate through the items
        if ((*it)->getID() == id) {  // Check if the current item's ID matches the specified ID
          destination.items.push_back(*it);  // Add item to destination container
          cout << "Chore moved successfully: " << (*it)->getName() << endl;
          items.erase(it);  // Remove item from the current container
          found = true;  // Set found flag
          break;  // Exit loop after moving the item
        }
      }

      if (!found) {
        cout << "Chore not found." << endl;  // Inform if the item wasn't found
      }
    }

    // Delete an item from the container based on its ID
    void deleteItem(int id) {
      for (auto it = items.begin(); it != items.end(); ++it) {  // Iterate through items
        if ((*it)->getID() == id) {  // Check if current item's ID matches the specified ID
          items.erase(it);  // Remove item from vector
          break;  // Exit loop after removing item
        }
      }
    }

    // Add an item to the container
    void push_back(const shared_ptr<T>& item) {
      items.push_back(item);  // Append the item to the vector
    }

    // Return the number of items in the container
    size_t size() const {
      return items.size();
    }

    // Provide access to items by index
    shared_ptr<T>& operator[](size_t index) {
      return items[index];  // Return reference to the item at the given index
    }

    // Display all items in the container
    void display() const {
      for (const auto& item : items) {  // Loop through all items
        cout << "Chore: " << item->getName() << " (ID: " << item->getID() << ")" << endl;  // Display each item's name and ID
      }
    }

    // Check if the container is empty
    bool empty() const {
      return items.empty();
    }

    // Clear all items from the container
    void clear() {
      items.clear();  // Clear the vector
    }

    // Provide access to the internal items vector
    const vector<shared_ptr<T>>& item() const {
      return items;
    }

    // Begin iterator for range-based loops
    auto begin() -> decltype(items.begin()) {
      return items.begin();
    }

    // End iterator for range-based loops
    auto end() -> decltype(items.end()) {
      return items.end();
    }

    // Const begin iterator for range-based loops over const containers
    auto begin() const -> decltype(items.begin()) const {
      return items.begin();
    }

    // Const end iterator for range-based loops over const containers
    auto end() const -> decltype(items.end()) const {
      return items.end();
    }
  };

  //*********************************************************************************

  // Definition of the ChoreDoer class, which manages a list of chores and related information
  class ChoreDoer {
  public:
    // Container to store assigned chores
    Container<Chore> assignedChores;

    // Constructor to initialize a ChoreDoer with a name
    ChoreDoer(const string& name) : name(name) {
      age = 0;  // Default age is set to 0
      choreAmount = 0;  // Initialize the count of assigned chores to 0
      totalEarnings = 0;  // Initialize total earnings from completed chores to 0
      assignedChores = Container<Chore>();  // Initialize the container for managing chores
    }

    // Method to assign a chore to the doer
    void assignChore(const shared_ptr<Chore>& chore) {
      assignedChores.push_back(chore);  // Add the chore to the container
      choreAmount++;  // Increment the count of assigned chores
    }

    // Getter method to retrieve the chore doer's name
    string getName() const {
      return name;
    }

    // Getter method to retrieve the total earnings accumulated from completed chores
    int getTotalEarnings() const {
      return totalEarnings;
    }

    // Getter method to retrieve the number of chores assigned to the doer
    int getChoreAmount() const {
      return choreAmount;
    }

    // Method to generate a formatted string displaying chore doer's details
    string printChoreDoer() const {
      string result = "Chore Doer: " + name + "\n";  // Start with the chore doer's name
      result += "Total Earnings: $" + to_string(totalEarnings) + "\n";  // Add total earnings
      result += "Chore Amount: " + to_string(choreAmount) + "\n";  // Add number of chores assigned
      return result;
    }

    // Method to start a chore based on its ID
    void startChore(int choreId) {
      for (auto& chore : assignedChores) {
        if (chore->getID() == choreId) {
          chore->startChore();  // Call startChore on the matching chore
          break;
        }
      }
    }

    // Method to complete a chore based on its ID
    void completeChore(int choreId) {
      for (auto& chore : assignedChores) {
        if (chore->getID() == choreId) {
          if (chore->GetStatus() == STATUS::IN_PROGRESS || chore->GetStatus() == STATUS::NOT_STARTED) {
            chore->completeChore();  // Complete the chore
            totalEarnings += chore->getEarnings();  // Update total earnings
            cout << "Chore " << chore->getName() << " completed. Total earnings now: $" << totalEarnings << endl;
            break;
          }
          else {
            cout << "Chore " << chore->getName() << " is already completed. No action taken." << endl;
          }
        }
      }
    }

    // Method to reset a chore based on its ID
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

    // Overloaded insertion operator to output the details of the ChoreDoer
    friend ostream& operator<<(ostream& os, const ChoreDoer& chDoer);

  private:
    string name;  // Name of the chore doer
    int choreAmount;  // Number of chores assigned to the doer
    int age;  // Age of the chore doer, not implemented in full
    int totalEarnings;  // Total earnings from chores completed by the doer
  };

  // Overloaded ostream operator to facilitate easy output of ChoreDoer's state
  ostream& operator<<(ostream& os, const ChoreDoer& chDoer) {
    os << chDoer.printChoreDoer();  // Output formatted chore doer details
    return os;
  }

  //*********************************************************************************
  class ChoreManager {
  public:
    // Constructor for ChoreManager that initializes its internal state with file data
    ChoreManager(string fileName) : client(json{}) {
      try {
        dynamicFile = fileName; // Save the filename for later use

        // Seed the random number generator for any randomized operations
        srand(static_cast<unsigned int>(time(nullptr)));

        ifstream file(fileName); // Open the specified file
        if (file.is_open()) {
          // Parse the contents of the file into a JSON object if not empty
          if (j.is_null()) {
            j = json::parse(file);
          }
          file.close(); // Always close the file after opening
        }
        else {
          cerr << "Failed to open file: " << fileName << endl; // Error message if file cannot be opened
        }

        // Initialize client with the user profile from the JSON data
        if (j.contains("user_profile")) {
          if (this->client.getUserName() == "DefaultUser") {
            this->client.setUsername();
          }
        }
        choreCount = 0; // Initialize the chore count
        loadChores(); // Load chores from the JSON object
      }
      catch (const json::exception& e) {
        cerr << "JSON Error: " << e.what() << endl; // Handle JSON parsing errors
        throw; // Rethrow the exception for external handling
      }
      catch (const ifstream::failure& e) {
        cerr << "File Error: " << e.what() << endl; // Handle file opening errors
        throw;
      }
    }

    // Destructor to clear resources
    ~ChoreManager() {
      clearAll();
    }

    // Method to load chores from the JSON object into the container
    void loadChores() {
      try {
        if (j.contains("chores") && j["chores"].is_array()) {
          for (auto& choreJson : j["chores"]) {
            string difficulty = choreJson.value("difficulty", "unknown"); // Get the difficulty level from the JSON
            shared_ptr<Chore> chore;

            // Create a specific type of Chore based on the difficulty level
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

            Chores.push_back(chore); // Add chore to the container
            choreCount++; // Increment the count of chores
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

    // Method to modify either user profile data or chore data interactively
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
      saveData(); // Save changes to file
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

    // Display all chores in the container
    void displayChores() const {
      for (const auto& chore : Chores) {
        cout << "Chore: " << chore->getName() << " (ID: " << chore->getID() << ")" << endl;
      }
    }

    // Display all chore assignments to each chore doer
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

    // Method to delete a chore from the available chore list by ID
    void deleteChoreFromAvailable(int choreId) {
      Chores.deleteItem(choreId);
    }

    // Method to delete a chore from leftover chores by ID
    void deleteLeftoverChore(int choreId) {
      LeftoverChores.deleteItem(choreId);
    }

    // Method to delete a chore from any chore doer's assigned list by ID
    void deleteChoreFromAnyDoer(int choreId) {
      for (auto& doer : ChoreDoers) {
        doer.assignedChores.deleteItem(choreId);
      }
    }

    // Method to add a new chore doer to the system
    void addChoreDoer(const string& name) {
      ChoreDoers.push_back(ChoreDoer(name));
    }

    // Method to delete a chore doer from the system by name
    void deleteChoreDoer(const string& name) {
      try {
        for (auto it = ChoreDoers.begin(); it != ChoreDoers.end(); ++it) {
          if (it->getName() == name) {
            ChoreDoers.erase(it);
            break;
          }
        }
      }
      catch (const exception& e) {
        cerr << "Exception thrown in removeChoreDoer: " << e.what() << endl;
      }
    }

    // Method to add a new chore to the system from a JSON object
    void addChore(const json& choreJson) {
      shared_ptr<Chore> chore;

      if (choreJson.is_null()) {
        cerr << "Invalid chore JSON." << endl;
        return;
      }
      if (choreJson.contains("difficulty") && choreJson["difficulty"].is_string()) {
        string difficulty = choreJson["difficulty"].get<string>();

        if (difficulty == "easy") {
          chore = make_shared<EasyChore>(choreJson);
        }
        else if (difficulty == "medium") {
          chore = make_shared<MediumChore>(choreJson);
        }
        else if (difficulty == "hard") {
          chore = make_shared<HardChore>(choreJson);
        }
      }
      Chores.push_back(chore);
      saveData(); // Save new chore and update data
    }

    // Method to move a chore to leftover chores list by ID
    void moveChoreToLeftover(int choreId) {
      for (auto& doer : ChoreDoers) {
        doer.assignedChores.moveItemToAnotherContainer(choreId, LeftoverChores);
      }
    }

    // Method to move a chore from leftover chores to a specific chore doer
    void moveChoreFromLeftoverToDoer(const string& toDoer, int choreId) {
      for (auto& doer : ChoreDoers) {
        if (doer.getName() == toDoer) {
          LeftoverChores.moveItemToAnotherContainer(choreId, doer.assignedChores);
          break;
        }
        else {
          cout << "Chore Doer not found." << endl;
        }
      }
    }

    // Method to move a chore between two chore doers
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

    // Method to assign chores randomly to chore doers
    void assignChoresRandomly() {
      try {
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

          // Handle leftover chores
          while (choreIndex < Chores.size()) {
            LeftoverChores.push_back(Chores[choreIndex++]);
          }
        }
      }
      catch (const exception& e) {
        cerr << "Exception thrown in assignChoresRandomly: " << e.what() << endl;
      }
    }

    // Method to create a minimal chore structure
    void createMinimalChore() {
      // Implementation would typically include setting essential fields only
    }

    // Method to create a full-fledged chore with all details
    void createFullChore() {
      // Basic chore details
      string name, description, frequency, estimated_time, difficulty, days, location,
        tools_required, materials_needed, priority, notes, status, tags, multitasking_tips,
        earningsInput, input;
      double earnings;

      // Variables for handling unique attributes (Easy, Medium, Hard)
      json choreJson;
      char choice;
      //Id
      choreJson["id"] = choreCount++;

      //Name
      cout << "Enter chore name: ";
      getline(cin, name);
      choreJson["name"] = name.empty() ? "" : name;

      //Description
      cout << "Enter chore description: ";
      getline(cin, description);
      choreJson["description"] = description.empty() ? "" : description;

      //Frequency
      cout << "Enter chore frequency (e.g., daily, weekly): ";
      getline(cin, frequency);
      choreJson["frequency"] = frequency.empty() ? "" : frequency;

      //Estimated_time
      cout << "Enter estimated time for chore completion: ";
      getline(cin, estimated_time);
      choreJson["estimated_time"] = estimated_time.empty() ? "" : estimated_time;

      // Handle earnings input with robust validation
      // If not entered correctly -> re-enter
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

      //Days
      cout << "Enter days when the chore should be performed (comma separated): ";
      getline(cin, days);
      choreJson["days"] = days.empty() ? json::array() : json::parse("[" + days + "]");

      //Location
      cout << "Enter chore location: ";
      getline(cin, location);
      choreJson["location"] = location.empty() ? json::array() : json::parse("[" + location + "]");

      //Tools_required
      cout << "Enter tools required (comma separated): ";
      getline(cin, tools_required);
      choreJson["tools_required"] = tools_required.empty() ? json::array() : json::parse("[" + tools_required + "]");

      //Materials_needed
      cout << "Enter materials needed (comma separated): ";
      getline(cin, materials_needed);
      choreJson["materials_needed"] = materials_needed.empty() ? json::array() : json::parse("[" + materials_needed + "]");

      //Notes
      cout << "Enter any additional notes: ";
      getline(cin, notes);
      choreJson["notes"] = notes.empty() ? "" : notes;

      //Tags
      cout << "Enter tags for the chore (comma separated): ";
      getline(cin, tags);
      choreJson["tags"] = tags.empty() ? json::array() : json::parse("[" + tags + "]");

      //Difficulty
      //If not entered correctly -> re-enter
      cout << "Enter difficulty (easy, medium, hard): ";
      getline(cin, difficulty);
      while (difficulty != "easy" && difficulty != "medium" && difficulty != "hard" && !difficulty.empty()) {
        cout << "Invalid difficulty. Please enter 'easy', 'medium', or 'hard': ";
        getline(cin, difficulty);
      }
      choreJson["difficulty"] = difficulty.empty() ? "" : difficulty;

      //Priority
      //If not entered correctly -> re-enter
      cout << "Enter priority (low, moderate, high): ";
      getline(cin, priority);
      while (priority != "low" && priority != "moderate" && priority != "high" && !priority.empty()) {
        cout << "Invalid priority. Please enter 'low', 'moderate', or 'high': ";
        getline(cin, priority);
      }
      choreJson["priority"] = priority.empty() ? "" : priority;

      //Status
      //If not entered correctly -> re-enter
      cout << "Enter status (not started, in progress, completed): ";
      getline(cin, status);
      while (status != "not started" && status != "in progress" && status != "completed" && !status.empty()) {
        cout << "Invalid status. Please enter 'not started', 'in progress', or 'completed': ";
        getline(cin, status);
      }
      choreJson["status"] = status.empty() ? "" : status;

      // Unique attributes based on difficulty
      // EASY
      if (difficulty == "easy") {
        cout << "Enter multitasking tips for easy chore: ";
        getline(cin, multitasking_tips);
        choreJson["multitasking_tips"] = multitasking_tips.empty() ? "" : multitasking_tips;
      }
      // MEDIUM
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
      // HARD
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

          // Save the rest of subtask
          subtask["name"] = subtaskName.empty() ? "" : subtaskName;
          subtask["estimated_time"] = subtaskTime.empty() ? "" : subtaskTime;
          subtask["earnings"] = to_string(subtaskEarnings).empty() ? 0 : subtaskEarnings;
          choreJson["subtasks"] = json::array({ subtask });
        }
        // If n is entered, create empty subtask
        else
        {
          cout << "No subtasks added." << endl;
          choreJson["subtasks"] = json::array(); // should be empty
        }
      }

      // Add the chore to the chore list and save file
      addChore(choreJson);
      cout << "Chore added successfully!" << endl;
    }

    //Function wrapper for templated sort function
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
    json j; // JSON object to store data
    int choreCount; // Counter for the number of chores
    Container<Chore> Chores; // Container to hold chores
    vector<ChoreDoer> ChoreDoers; // List of chore doers
    Container<Chore> LeftoverChores; // Container for leftover chores
    Client client; // Client object, part of the ChoreManager
    string dynamicFile; // Filename for dynamic operations

    // Clear all data from ChoreManager
    void clearAll() {
      try {
        // Clear original Container of chores
        clearChores();
        // Clear leftover Container of chores
        clearLeftoverChores();
        // Clear assigned Container of chores for each chore doer
        clearAllAssignedChores();
        // Clear chore doers
        clearAllChoreDoers();
      }
      catch (const exception& e) {
        cerr << "Exception caught in clearChores: " << e.what() << endl;
      }
    }

    // Clear leftover chores from the container
    void clearLeftoverChores() {
      try {
        if (!LeftoverChores.empty()) {
          LeftoverChores.clear();
        }
      }
      catch (const exception& e) {
        cerr << "Exception caught in clearLeftoverChores: " << e.what() << endl;
      }
    }

    // Clear all chores from the container
    void clearChores() {
      try {
        if (!Chores.empty()) {
          Chores.clear();
        }
      }
      catch (const exception& e) {
        cerr << "Exception caught in DeleteChores: " << e.what() << endl;
      }
    }

    // Clear all chore doers from the list
    void clearChoreDoers() {
      try {
        ChoreDoers.clear();
      }
      catch (const exception& e) {
        cerr << "Exception caught in clearChoreDoers: " << e.what() << endl;
      }
    }

    // Clear all assigned chores for each chore doer
    void clearAllAssignedChores() {
      if (!ChoreDoers.empty()) {
        // Clear assigned Container of chores for each chore doer
        for (auto& doer : ChoreDoers) {
          if (!doer.assignedChores.empty()) {
            doer.assignedChores.clear();
          }
        }
      }
    }

    // Clear all chore doers from the list
    void clearAllChoreDoers() {
      // Clear chore doers
      if (!ChoreDoers.empty()) {
        ChoreDoers.clear();
      }
    }
  };

  int main() {
    try {
      string testFile(DATA_FILE_PATH + "data.json");
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