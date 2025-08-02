#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<ctime>
#include<iomanip>
#include <chrono>
#include <thread>
using namespace std;
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// ------------------ Utility ------------------
void safecopy(char* dest, const char* src, int maxLen)
{
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1)
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

string compressRLE(const string& input)
{
    char result[1000];
    int resultIndex = 0;

    int i = 0;
    while (input[i] != '\0')
    {
        char currentChar = input[i];
        int count = 1;
        while (input[i + 1] != '\0' && input[i + 1] == currentChar)
        {
            count++;
            i++;
        }
        int tempCount = count;
        char countDigits[10];
        int digitCount = 0;
        while (tempCount > 0)
        {
            countDigits[digitCount] = (tempCount % 10) + '0';
            tempCount = tempCount / 10;
            digitCount++;
        }
        for (int j = digitCount - 1; j >= 0; j--)
        {
            result[resultIndex] = countDigits[j];
            resultIndex++;
        }
        result[resultIndex] = currentChar;
        resultIndex++;
        i++;
    }
    result[resultIndex] = '\0';
    return string(result);
}
string decompressRLE(const string& input)
{
    char result[1000];
    int resultIndex = 0;

    int i = 0;
    while (input[i] != '\0')
    {
        int count = 0;
        while (input[i] >= '0' && input[i] <= '9')
        {
            count = count * 10 + (input[i] - '0');
            i++;
        }
        char currentChar = input[i];
        i++;
        for (int j = 0; j < count; j++)
        {
            result[resultIndex] = currentChar;
            resultIndex++;
        }
    }
    result[resultIndex] = '\0';
    return string(result);
}

bool haspermission(const char* action, const char* role)
{
    if (strcmp(role, "Admin") == 0)
    {
        return true;
    }
    if (strcmp(role, "Editor") == 0)
    {
        return strcmp(action, "execute") != 0;
    }
    if (strcmp(role, "Viewer") == 0)
    {
        return strcmp(action, "read") == 0;
    }
    return false;
}

// ------------------ Cloud Sync ------------------
struct SyncNode
{
    char filename[50];
    SyncNode* next;
};

SyncNode* syncfront = nullptr;
SyncNode* syncrear = nullptr;

void enqueueSync(const char* name)
{
    SyncNode* temp = new SyncNode();
    safecopy(temp->filename, name, 50);
    temp->next = nullptr;
    if (!syncrear)
    {
        syncfront = syncrear = temp;
    }
    else
    {
        syncrear->next = temp;
        syncrear = temp;
    }
}

void processSync()
{
    cout << "Syncing to cloud..." << endl;
    while (syncfront)
    {
        cout << "Synced file : " << syncfront->filename << endl;
        SyncNode* temp = syncfront;
        syncfront = syncfront->next;
        delete temp;
    }
    syncrear = nullptr;
}

// ------------------ AVL Tree (Stub for Scalability) ------------------
class AVLNode
{
public:
    char name[50];
    AVLNode* left;
    AVLNode* right;
    int height;

    AVLNode(const char* n)
    {
        safecopy(name, n, 50);
        left = right = nullptr;
        height = 1;
    }
};

AVLNode* rootAVL = nullptr;

int height(AVLNode* node)
{
    return node ? node->height : 0;
}

int getBalance(AVLNode* node)
{
    return node ? height(node->left) - height(node->right) : 0;
}

AVLNode* rightRotate(AVLNode* y)
{
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

AVLNode* leftRotate(AVLNode* x)
{
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}


// ------------------ Directory Structure ------------------
class filenode;

class DirectoryNode
{
public:
    char name[50];
    DirectoryNode* parent;
    DirectoryNode* firstChild;
    DirectoryNode* nextSibling;
    filenode* files;

    DirectoryNode(const char* dirname, DirectoryNode* parentDir = NULL)
    {
        safecopy(name, dirname, 50);
        parent = parentDir;
        firstChild = NULL;
        nextSibling = NULL;
        files = NULL;
    }
};

// Root directory
DirectoryNode* rootDirectory = NULL;
DirectoryNode* currentDirectory = NULL;

// ------------------ User Management ------------------
class User
{
public:
    int id;
    char username[50];
    char password[50];
    char secQuestion[100];
    char secAnswer[100];
    char role[10];
    char lastLogin[30];
    char lastLogout[30];
    int usedStorageMB;
    static const int MAX_STORAGE_MB = 1000;

    User(int uid, const char* uname, const char* pwd, const char* question, const char* answer, const char* userRole)
    {
        id = uid;
        safecopy(username, uname, 50);
        safecopy(password, pwd, 50);
        safecopy(secQuestion, question, 100);
        safecopy(secAnswer, answer, 100);
        safecopy(role, userRole, 10);
        usedStorageMB = 0;
        lastLogin[0] = '\0';
        lastLogout[0] = '\0';
    }
};

User* users[10];
int userCount = 0;
int currentuserid = -1;
const int MAX_USERS = 10;
int shareGraph[MAX_USERS][MAX_USERS] = { 0 };

void saveUsersToFile()
{
    ofstream fout("user_s.txt");
    for (int i = 0; i < userCount; i++)
    {
        fout << users[i]->id << endl << users[i]->username << endl
            << users[i]->password << endl << users[i]->secQuestion << endl
            << users[i]->secAnswer << endl << users[i]->role << endl
            << users[i]->usedStorageMB << endl
            << users[i]->lastLogin << endl << users[i]->lastLogout << endl;
    }
    fout.close();
}

void loadUsersFromFile()
{
    ifstream fin("user_s.txt");
    if (!fin.is_open())
    {
        return;
    }
    while (!fin.eof())
    {
        int id, used;
        char uname[50], pwd[50], ques[100], ans[100], role[10], lastLogin[30], lastLogout[30];
        fin >> id;
        if (fin.eof())
        {
            break;
        }
        fin.ignore();
        fin.getline(uname, 50);
        fin.getline(pwd, 50);
        fin.getline(ques, 100);
        fin.getline(ans, 100);
        fin.getline(role, 10);
        fin >> used;
        fin.ignore();
        fin.getline(lastLogin, 30);
        fin.getline(lastLogout, 30);

        users[userCount] = new User(id, uname, pwd, ques, ans, role);
        users[userCount]->usedStorageMB = used;
        safecopy(users[userCount]->lastLogin, lastLogin, 30);
        safecopy(users[userCount]->lastLogout, lastLogout, 30);
        userCount++;
    }
    fin.close();
}

void addUser()
{
    if (userCount >= 10)
    {
        cout << "User limit has reached." << endl;
        return;
    }
    char uname[50], pwd[50], ques[100], ans[100], role[10];
    cout << "Enter username : "; cin >> uname;
    cout << "Enter password : "; cin >> pwd;
    cin.ignore();
    cout << "Enter security question : "; cin.getline(ques, 100);
    cout << "Enter answer to security question : "; cin.getline(ans, 100);
    cout << "Choose role (Admin/Editor/Viewer) : "; cin >> role;
    users[userCount] = new User(userCount, uname, pwd, ques, ans, role);
    userCount++;
    saveUsersToFile();
    cout << "User registered successfully." << endl;
}

void login()
{
    char uname[50], pwd[50];
    cout << "Enter username: "; cin >> uname;
    cout << "Enter password: "; cin >> pwd;
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i]->username, uname) == 0 && strcmp(users[i]->password, pwd) == 0)
        {
            currentuserid = i;
            cout << "Login successful as " << users[i]->role << "\n";
            if (users[i]->lastLogout[0] != '\0') 
            {
                cout << "Last logout: " << users[i]->lastLogout << endl;
            }
            return;
        }
    }
    cout << "Invalid username or password." << endl;
}

void recoverAccount()
{
    char uname[50], answer[100];
    cout << "Enter your username: "; 
    cin >> uname;
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i]->username, uname) == 0)
        {
            cout << "Security Question: " << users[i]->secQuestion << endl;
            cin.ignore();
            cout << "Answer: "; cin.getline(answer, 100);
            if (strcmp(users[i]->secAnswer, answer) == 0)
            {
                cout << "Your password is: " << users[i]->password << endl;
                return;
            }
            else
            {
                cout << "Incorrect answer." << endl;
                return;
            }
        }
    }
    cout << "User not found." << endl;
}

// ------------------ File Management ------------------
class FileVersion
{
public:
    char content[100];
    FileVersion* next;
};

class filenode
{
public:
    char name[50];
    char content[100];
    char access_role[10];
    FileVersion* history;
    filenode* next;

    filenode(const char* fname, const char* fcontent, const char* access)
    {
        safecopy(name, fname, 50);
        safecopy(content, fcontent, 100);
        safecopy(access_role, access, 10);
        history = NULL;
        next = NULL;
    }
};

filenode* files = NULL;

// ------------------ Hash Table for File Lookup ------------------
const int Hash_Size = 100;
filenode* hashTable[Hash_Size] = { NULL };

int hashFunction(const char* name)
{
    int hash = 0;
    for (int i = 0; name[i]; i++) hash = (hash * 31 + name[i]) % Hash_Size;
    return hash;
}

void insertHash(filenode* file)
{
    int index = hashFunction(file->name);
    file->next = hashTable[index];
    hashTable[index] = file;
}

filenode* searchHash(const char* name)
{
    int index = hashFunction(name);
    filenode* temp = hashTable[index];
    while (temp)
    {
        if (strcmp(temp->name, name) == 0)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void searchEdge(int fromUser, int toUser)
{
    shareGraph[fromUser][toUser] = 1;
}

bool bfsShare(int fromUser, int toUser)
{
    bool visited[MAX_USERS] = { false };
    int queue[MAX_USERS], front = 0, rear = 0;
    queue[rear++] = fromUser;
    visited[fromUser] = true;
    while (front < rear)
    {
        int u = queue[front++];
        if (u == toUser)
        {
            return true;
        }
        for (int v = 0; v < userCount; v++)
        {
            if (shareGraph[u][v] && !visited[v])
            {
                visited[v] = true;
                queue[rear++] = v;
            }
        }
    }
    return false;
}

// ------------------ Recycle Bin ------------------
class StackNode
{
public:
    char name[50];
    StackNode* next;
};
StackNode* bin_top = nullptr;

void movetorecyclebin(const char* filename)
{
    StackNode* temp = new StackNode();
    safecopy(temp->name, filename, 50);
    temp->next = bin_top;
    bin_top = temp;
}

void popfromrecyclebin()
{
    if (!bin_top)
    {
        cout << "Recycle Bin is empty." << endl;
        return;
    }
    cout << "Recovered file: " << bin_top->name << endl;
    StackNode* temp = bin_top;
    bin_top = bin_top->next;
    delete temp;
}

// ------------------ Recent Files Queue ------------------
class QueueNode
{
public:
    char name[50];
    QueueNode* next;
};

QueueNode* qfront = nullptr;
QueueNode* qrear = nullptr;
int queuesize = 0;
const int queue_capacity = 5;

void enqueueRecent(const char* name)
{
    QueueNode* temp = new QueueNode();
    safecopy(temp->name, name, 50);
    temp->next = nullptr;
    if (!qrear)
    {
        qfront = qrear = temp;
    }
    else
    {
        qrear->next = temp;
        qrear = temp;
    }
    queuesize++;
    if (queuesize > queue_capacity)
    {
        QueueNode* del = qfront;
        qfront = qfront->next;
        delete del;
        queuesize--;
    }
}

void showRecent()
{
    QueueNode* temp = qfront;
    cout << "Recent Files : ";
    while (temp)
    {
        cout << temp->name << " ";
        temp = temp->next;
    }
    cout << endl;
}

// ------------------ File System Persistence ------------------
void saveFileToSystem(const char* fname, const char* owner, const char* content)
{
    ofstream fout("filesystem.txt", ios::app);
    if (!fout.is_open())
    {
        cout << "Error opening file." << endl;
        return;
    }
    fout << fname << "|" << owner << "|" << content << endl;
    fout.close();
}

void saveSharedAccess(const char* fname, const char* owner, const char* sharedUser, const char* role)
{
    ofstream fout("sharedfile.txt", ios::app);
    fout << fname << " " << owner << " " << sharedUser << " " << role << endl;
    fout.close();
}

void loadFilesFromSystem()
{
    ifstream infile("filesystem.txt");
    if (!infile)
    {
        cout << "No saved files found." << endl;
        return;
    }
    char line[256];
    while (infile.getline(line, 256))
    {
        char fname[50] = "", owner[50] = "", content[100] = "";
        int i = 0, k = 0;
        while (line[i] != '|' && line[i] != '\0')
        {
            fname[k++] = line[i++];
        }
        fname[k] = '\0';
        if (line[i] == '\0')
        {
            continue;
        }
        i++;
        k = 0;
        while (line[i] != '|' && line[i] != '\0')
        {
            owner[k++] = line[i++];
        }
        owner[k] = '\0';
        if (line[i] == '\0') continue;
        i++;
        k = 0;
        while (line[i] != '\0')
        {
            content[k++] = line[i++];
        }
        content[k] = '\0';

        filenode* newfile = new filenode(fname, content, "user");
        insertHash(newfile);
        newfile->next = files;
        files = newfile;
    }
    infile.close();
}

// ------------------ File Operations ------------------
void createFile()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    if (!haspermission("write", users[currentuserid]->role)) 
    {
        cout << "Permission Denied : You do not have write access." << endl;
        return;
    }
    char fname[50];
    string content;
    cout << "Enter file name : "; cin >> fname;
    cin.ignore();
    cout << "Enter file content : "; getline(cin, content);

    string compressed = compressRLE(content);
    filenode* newfile = new filenode(fname, compressed.c_str(), users[currentuserid]->role);
    newfile->next = files;
    files = newfile;
    insertHash(newfile);
    if (currentDirectory)
    {
        filenode* dirFile = new filenode(fname, compressed.c_str(), users[currentuserid]->role);
        dirFile->next = currentDirectory->files;
        currentDirectory->files = dirFile;
    }
    enqueueRecent(fname);
    saveFileToSystem(fname, users[currentuserid]->username, compressed.c_str());
    enqueueSync(fname);

    cout << "File created and compressed successfully." << endl;
}

void readfile()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    char fname[50];
    cout << "Enter file name to read : "; cin >> fname;
    filenode* temp = searchHash(fname);
    if (!temp)
    {
        cout << "File not found." << endl;
        return;
    }
    if (!haspermission("read", users[currentuserid]->role))
    {
        cout << "Permission Denied: You do not have read access." << endl;
        return;
    }
    string decompressed = decompressRLE(temp->content);
    cout << "Content: " << decompressed << endl;
    enqueueRecent(fname);
}

void updateFile()
{
    if (currentuserid == -1 || !haspermission("write", users[currentuserid]->role))
    {
        cout << "Permission Denied." << endl;
        return;
    }
    char fname[50];
    string newcontent;
    cout << "Enter file name to update: "; cin >> fname;
    filenode* temp = searchHash(fname);
    if (!temp)
    {
        cout << "File not found." << endl;
        return;
    }
    if (!haspermission("write", users[currentuserid]->role))
    {
        cout << "Permission Denied: You do not have write access." << endl;
        return;
    }
    FileVersion* newVersion = new FileVersion();
    safecopy(newVersion->content, temp->content, 100);
    newVersion->next = temp->history;
    temp->history = newVersion;
    cin.ignore();
    cout << "Enter new content: "; 
    getline(cin, newcontent);
    string compressed = compressRLE(newcontent);
    safecopy(temp->content, compressed.c_str(), 100);
    enqueueRecent(fname);
    enqueueSync(fname);
    saveFileToSystem(fname, users[currentuserid]->username, compressed.c_str());

    cout << "File updated and compressed. Previous version saved." << endl;
}

void rollbackFileVersion()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    char fname[50];
    cout << "Enter file name to rollback: "; cin >> fname;
    filenode* temp = searchHash(fname);
    if (!temp)
    {
        cout << "File not found." << endl;
        return;
    }
    if (!haspermission("write", users[currentuserid]->role))
    {
        cout << "Permission Denied: You do not have write access." << endl;
        return;
    }
    if (temp->history == NULL)
    {
        cout << "No previous versions available." << endl;
        return;
    }
    FileVersion* currentVersion = new FileVersion();
    safecopy(currentVersion->content, temp->content, 100);
    FileVersion* toRollback = temp->history;
    safecopy(temp->content, toRollback->content, 100);
    temp->history = toRollback->next;
    delete toRollback;
    currentVersion->next = temp->history;
    temp->history = currentVersion;
    cout << "Rolled back to previous version." << endl;
    enqueueSync(fname);
}

void shareFile()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    char fname[50], sharedTo[50], role[10];
    cout << "Enter file name to share : ";
    cin >> fname;
    cout << "Enter username to share with : "; 
    cin >> sharedTo;
    cout << "Enter access level (Editor/Viewer) : "; 
    cin >> role;
    filenode* temp = searchHash(fname);
    if (!temp)
    {
        cout << "File not found." << endl;
        return;
    }
    int userid = -1;
    for (int i = 0; i < userCount; i++)
    {
        if (strcmp(users[i]->username, sharedTo) == 0)
        {
            userid = i;
            break;
        }
    }
    if (userid == -1)
    {
        cout << "User not found." << endl;
        return;
    }
    searchEdge(currentuserid, userid);
    saveSharedAccess(fname, users[currentuserid]->username, sharedTo, role);
    cout << "File shared successfully." << endl;
}

void deleteFile()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    if (!haspermission("write", users[currentuserid]->role))
    {
        cout << "Permission Denied: You do not have delete access." << endl;
        return;
    }
    char fname[50];
    cout << "Enter file name to delete : "; cin >> fname;
    filenode* filedel = searchHash(fname);
    if (!filedel)
    {
        cout << "File not found." << endl;
        return;
    }
    movetorecyclebin(fname);
    filenode* curr = files, * prev = NULL;
    while (curr)
    {
        if (strcmp(curr->name, fname) == 0)
        {
            if (prev)
            {
                prev->next = curr->next;
            }
            else
            {
                files = curr->next;
            }
            int hashIndex = hashFunction(fname);
            filenode* hashCurr = hashTable[hashIndex];
            filenode* hashPrev = NULL;
            while (hashCurr) 
            {
                if (strcmp(hashCurr->name, fname) == 0) 
                {
                    if (hashPrev) 
                    {
                        hashPrev->next = hashCurr->next;
                    }
                    else 
                    {
                        hashTable[hashIndex] = hashCurr->next;
                    }
                    break;
                }
                hashPrev = hashCurr;
                hashCurr = hashCurr->next;
            }
            if (currentDirectory)
            {
                filenode* dirCurr = currentDirectory->files;
                filenode* dirPrev = NULL;
                while (dirCurr)
                {
                    if (strcmp(dirCurr->name, fname) == 0)
                    {
                        if (dirPrev)
                        {
                            dirPrev->next = dirCurr->next;
                        }
                        else
                        {
                            currentDirectory->files = dirCurr->next;
                        }
                        delete dirCurr;
                        break;
                    }
                    dirPrev = dirCurr;
                    dirCurr = dirCurr->next;
                }
            }
            enqueueSync(fname);
            cout << "File moved to Recycle Bin." << endl;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

bool isSubstring(char* text, char* pattern)
{
    int i = 0, j = 0;
    while (text[i] != '\0')
    {
        j = 0;
        int k = i;
        while (pattern[j] != '\0' && text[k] == pattern[j])
        {
            j++;
            k++;
        }
        if (pattern[j] == '\0')
        {
            return true;
        }
        i++;
    }
    return false;
}

void searchFile()
{
    if (currentuserid == -1)
    {
        cout << "Login required." << endl;
        return;
    }
    char query[50];
    cout << "Enter file name to search: ";
    cin >> query;
    filenode* temp = files;
    bool found = false;
    while (temp)
    {
        if (isSubstring(temp->name, query))
        {
            cout << GREEN << "Found file: " << temp->name << RESET << endl;
            found = true;
        }
        temp = temp->next;
    }
    if (!found)
    {
        cout << RED << "No matching files found." << RESET << endl;
    }
}

// ------------------ Directory Functions ------------------
void listDirectory()
{
    if (!currentDirectory)
    {
        cout << "Directory system not initialized." << endl;
        return;
    }
    cout << "Current directory: " << currentDirectory->name << endl;
    cout << "Subdirectories:" << endl;
    DirectoryNode* subdir = currentDirectory->firstChild;
    int count = 0;
    while (subdir)
    {
        cout << "  " << subdir->name << "/" << endl;
        subdir = subdir->nextSibling;
        count++;
    }
    if (count == 0) cout << "  (none)" << endl;
    cout << "Files:" << endl;
    filenode* filenode = currentDirectory->files;
    count = 0;
    while (filenode)
    {
        cout << "  " << filenode->name << endl;
        filenode = filenode->next;
        count++;
    }
    if (count == 0) cout << "  (none)" << endl;
}

void createDirectory()
{
    if (!currentDirectory)
    {
        rootDirectory = new DirectoryNode("root");
        currentDirectory = rootDirectory;
        cout << "Root directory initialized." << endl;
        return;
    }
    char dirname[50];
    cout << "Enter directory name: "; cin >> dirname;
    DirectoryNode* newDir = new DirectoryNode(dirname, currentDirectory);
    if (!currentDirectory->firstChild)
    {
        currentDirectory->firstChild = newDir;
    }
    else
    {
        DirectoryNode* temp = currentDirectory->firstChild;
        while (temp->nextSibling)
        {
            temp = temp->nextSibling;
        }
        temp->nextSibling = newDir;
    }
    cout << "Directory created: " << dirname << endl;
}

void changeDirectory()
{
    if (!currentDirectory)
    {
        cout << "Directory system not initialized. Creating root directory." << endl;
        rootDirectory = new DirectoryNode("root");
        currentDirectory = rootDirectory;
        return;
    }
    char dirname[50];
    cout << "Enter directory name (.. to go up): "; cin >> dirname;
    if (strcmp(dirname, "..") == 0)
    {
        if (currentDirectory->parent)
        {
            currentDirectory = currentDirectory->parent;
            cout << "Changed to parent directory: " << currentDirectory->name << endl;
        }
        else
        {
            cout << "Already at root directory." << endl;
        }
        return;
    }
    DirectoryNode* subdir = currentDirectory->firstChild;
    while (subdir)
    {
        if (strcmp(subdir->name, dirname) == 0)
        {
            currentDirectory = subdir;
            cout << "Changed directory to: " << dirname << endl;
            return;
        }
        subdir = subdir->nextSibling;
    }
    cout << "Directory not found." << endl;
}

// ------------------ Menu ------------------
void showMenu()
{
    int choice;
    while (true)
    {
        cout << BOLD << CYAN;
        cout << "+------------------------------------------------------------+\n";
        cout << "|                   FILE MANAGEMENT SYSTEM                   |\n";
        cout << "+------------------------------------------------------------+\n";
        cout << "|" << RESET << BLUE << " 1. Create File           " << RESET << RED << "| 9. Rollback File Version        " << CYAN << "|\n";
        cout << "|" << RESET << BLUE << " 2. Read File             " << RESET << RED << "|10. Check File Sharing Access    " << CYAN << "|\n";
        cout << "|" << RESET << BLUE << " 3. Update File           " << RESET << RED << "|11. Sync with Cloud              " << CYAN << "|\n";
        cout << "|" << RESET << BLUE << " 4. Delete File           " << RESET << RED << "|12. List Directory              " << CYAN << " |\n";
        cout << "|" << RESET << GREEN << " 5. Recover Deleted File  " << RESET << YELLOW << "|13. Change Directory            " << CYAN << " |\n";
        cout << "|" << RESET << GREEN << " 6. Show Recent Files     " << RESET << YELLOW << "|14. Create Directory            " << CYAN << " |\n";
        cout << "|" << RESET << GREEN << " 7. Share File            " << RESET << YELLOW << "|15. logout                      " << CYAN << " |\n";
        cout << "|" << RESET << GREEN << " 8. Search File           " << RESET << YELLOW << "|                                " << CYAN << " |\n";
        cout << "+------------------------------------------------------------+\n";
        cout << RESET << BOLD << "Enter choice: " << RESET;
        if (!(cin >> choice))
        {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        switch (choice)
        {
        case 1:
            createFile();
            break;
        case 2:
            readfile();
            break;
        case 3:
            updateFile();
            break;
        case 4:
            deleteFile();
            break;
        case 5:
            popfromrecyclebin();
            break;
        case 6:
            showRecent();
            break;
        case 7:
            shareFile();
            break;
        case 8:
            searchFile();
            break;
        case 9:
            rollbackFileVersion();
            break;
        case 10:
        {
            int uid;
            cout << "Enter user ID to check if shared : ";
            cin >> uid;
            if (bfsShare(currentuserid, uid)) cout << "Sharing path exists." << endl;
            else cout << "No sharing path found." << endl;
            break;
        }
        case 11:
            processSync();
            break;
        case 12:
            listDirectory();
            break;
        case 13:
            changeDirectory();
            break;
        case 14:
            createDirectory();
            break;
        case 15:
            if (currentuserid != -1)
            {
                saveUsersToFile();
            }
            currentuserid = -1;
            cout << "Logged out." << endl;
            return;
        default: cout << "Invalid choice." << endl;
        }
    }
}

// ------------------ Main ------------------
int main()
{
    cout << "\033[2J\033[1;1H";
    for (int i = 0; i < 9; i++)
    {
        cout << "\n";
    }
    string pad = "               ";

    cout << BOLD << GREEN;
    cout << pad << " GGGGG    OOOOO   OOOOO   GGGGG   L       EEEEE    DDDD   RRRR   III  V   V  EEEEE \n";
    cout << pad << " G        O   O   O   O   G       L       E        D   D  R   R   I   V   V  E     \n";
    cout << pad << " G  GG    O   O   O   O   G  GG   L       EEEE     D   D  RRRR    I   V   V  EEEE  \n";
    cout << pad << " G   G    O   O   O   O   G   G   L       E        D   D  R  R    I    V V   E     \n";
    cout << pad << "  GGGG    OOOOO   OOOOO    GGGG   LLLLL   EEEEE    DDDD   R   R  III    V    EEEEE \n";
    cout << RESET;
    this_thread::sleep_for(chrono::seconds(3));

    cout << "\033[2J\033[1;1H";
    cout << RESET << YELLOW;
    cout << "==============================================================\n";
    cout << "|         Welcome to Google Drive File Management System     |\n";
    cout << "==============================================================\n";
    cout << RESET;
    cout << CYAN << "          Save, Share, and Manage your files easily!" << RESET << endl;
    cout << YELLOW << "               Made by: UMAR | HASSAAN | ZAIN" << RESET << endl;
    cout << "--------------------------------------------------------------\n\n";

    rootDirectory = new DirectoryNode("root");
    currentDirectory = rootDirectory;

    loadFilesFromSystem();
    loadUsersFromFile();

    int choice;
    while (true)
    {
        cout << BOLD << CYAN;
        cout << "+------------------------------------------+\n";
        cout << "|         Welcome to Google Drive          |\n";
        cout << "+------------------------------------------+\n";
        cout << "| " << RESET << BLUE << "1. Sign Up                               " << BOLD << CYAN << "|\n";
        cout << "| " << RESET << YELLOW << "2. Login                                 " << BOLD << CYAN << "|\n";
        cout << "| " << RESET << GREEN << "3. Recover Account                       " << BOLD << CYAN << "|\n";
        cout << "| " << RESET << RED << "4. Exit                                  " << BOLD << CYAN << "|\n";
        cout << "+------------------------------------------+\n";
        cout << RESET << BOLD << "Enter choice: " << RESET;
        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        switch (choice)
        {
        case 1:
            addUser();
            break;
        case 2:
            login();
            if (currentuserid != -1)
            {
                showMenu();
            }
            break;
        case 3:
            recoverAccount();
            break;
        case 4:
            saveUsersToFile();
            cout << "Thank you for using Google Drive File System!" << endl;
            return 0;
        default:
            cout << "Invalid choice." << endl;
            break;
        }
    }
}