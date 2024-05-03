# ITCS-2550-final-project
ITCS 2550 Final C++ Project for Group 1
# Statement of Work - Household Tasks Manager

This branch contains the Statement of Work (SoW) document for the Household Tasks Manager project developed by Group 1. The SoW outlines the project's scope, objectives, and detailed breakdown of responsibilities and requirements.

## Project Overview

The Household Tasks Manager is a software application designed to help users manage and schedule household chores effectively. It features user authentication, task scheduling, and notifications.

## Objectives

The main objectives of this project are:
- To provide a clear and intuitive user interface for managing tasks.
- To ensure data is stored and retrieved efficiently.
- To implement robust error handling and data validation.

## Key Features

- User account management
- Task creation, modification, and deletion
- Recurring tasks and notifications
- Data persistence through file I/O

## Group Members

- LaDawn Stuben - GUI Design and Implementation
- Philip Seros - Task Management Logic
- Wayne Williams - Data Persistence

## Instructor

Please review the attached Statement of Work document and provide your feedback or approval so that we may proceed with the project development.

- GitHub Username: @JohnAtMacomb
- Email: kossjo@macomb.edu

## Contribution

For group members, please ensure you follow the guidelines for contributing to this branch:
- Review the SoW document carefully.
- Provide feedback or suggest changes through pull request comments.
- Push any updates or corrections to this branch only.

### Application Documentation

1. **Client**
   - **Purpose**: Manages a user's profile within the application.
   - **Key Responsibilities**:
     - Stores user profile information, including username, login timestamps, and user preferences (e.g., notifications, themes).
     - Provides methods to modify the user profile, such as updating the username, preferences, and viewing profile details.
     - Serializes user profile data into JSON format for storage or transmission.

2. **Chore**
   - **Purpose**: Represents a single chore task.
   - **Key Responsibilities**:
     - Holds details about the chore, such as name, description, frequency, location, tools required, and rewards.
     - Manages chore status (not started, in progress, completed) and priority levels (low, moderate, high).
     - Provides methods to start, complete, or reset the chore's progress.
     - Allows modification of chore details and serialization of chore data into JSON.

3. **EasyChore** (derived from Chore)
   - **Purpose**: Specializes the Chore class for chores categorized as "easy".
   - **Key Responsibilities**:
     - Inherits all responsibilities of the Chore class.
     - Includes additional multitasking tips specific to easy chores.
     - Overrides start and complete methods to include behavior specific to easy chores, such as displaying multitasking tips.

4. **MediumChore** (derived from Chore)
   - **Purpose**: Represents chores categorized as "medium" difficulty.
   - **Key Responsibilities**:
     - Inherits all responsibilities of the Chore class.
     - Handles variations of the chore, which may include different ways or methods to complete the chore.
     - Overrides the start method to display available variations.

5. **HardChore** (derived from Chore)
   - **Purpose**: Categorizes chores that are "hard" and potentially consist of multiple subtasks.
   - **Key Responsibilities**:
     - Inherits all responsibilities of the Chore class.
     - Manages a list of subtasks, each with its own name, estimated completion time, and rewards.
     - Overrides the start method to detail each subtask involved in the chore.

6. **Container\<T>**
   - **Purpose**: A generic container for storing objects of type T (where T can be any object, like Chore).
   - **Key Responsibilities**:
     - Stores a collection of objects in a vector.
     - Provides methods to add, delete, and move objects within the container or to another container.
     - Facilitates sorting of the contained objects based on custom comparison functions.

7. **ChoreDoer**
   - **Purpose**: Represents an individual responsible for completing chores.
   - **Key Responsibilities**:
     - Manages a list of assigned chores.
     - Tracks total earnings and the number of chores assigned.
     - Provides methods to start, complete, or reset assigned chores and reports changes in chore status.

8. **ChoreManager**
   - **Purpose**: Central management class for chore handling within the application.
   - **Key Responsibilities**:
     - Initializes and manages user profiles and chore assignments.
     - Loads and saves chore data to and from a JSON format.
     - Provides a high-level interface for chore assignments, modifications, and reporting.
