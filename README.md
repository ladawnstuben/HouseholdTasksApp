# ITCS-2550-final-project
ITCS 2550 Final C++ Project for Group 1

# Household Tasks Manager - Project Overview

The Household Tasks Manager is a software application developed by Group 1 that helps users manage and schedule household chores effectively. This project includes features like user authentication, task scheduling, and notifications.

## Objectives

The main objectives of this project are:
- To provide a clear and intuitive user interface for managing tasks.
- To ensure data is stored and retrieved efficiently.
- To implement robust error handling and data validation.

## Key Features

- User account management
- Recurring tasks and notifications
- Data persistence through file I/O

## Group Members

- LaDawn Stuben - GUI Design and Implementation
- Philip Seros - Task Management Logic
- Wayne Williams - Data Persistence

## Instructor Contact

- GitHub Username: @JohnAtMacomb
- Email: kossjo@macomb.edu

## Installation Instructions

### Prerequisites

Before cloning the repository, you need to install Git Large File Storage (LFS) to manage large wxWidgets `.lib` files.

1. **Install Git LFS**:
    - **Windows**:
      - Download and install Git LFS from [git-lfs.github.com](https://git-lfs.github.com).
    - **macOS**:
      - Install Git LFS via Homebrew:  
        ```bash
        brew install git-lfs
        ```
    - **Linux**:
      - Install Git LFS using your distribution's package manager (e.g., `apt`, `yum`, etc.) or download it directly from the Git LFS website.

2. **Enable Git LFS**:
    - After installation, enable Git LFS globally:
    ```bash
    git lfs install
    ```

3. **Clone the Repository**:
    - Clone the repository to your local machine. The `.lib` files required for wxWidgets will automatically be downloaded:
    ```bash
    git clone https://github.com/ladawnstuben/ITCS-2550-final-project.git
    ```

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
