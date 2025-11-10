#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

#ifdef _WIN32
#define CLEAR_COMMAND "cls"
#else
#define CLEAR_COMMAND "clear"
#endif

using namespace std;

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct Position { 
    int x, y; 
    Position(int x=0, int y=0) : x(x), y(y) {} 
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class GameField {
private:
    int width=0, height=0;
    vector<vector<char>> grid;
    vector<vector<char>> baseGrid;
    Position dinosaurPos;
    bool dinosaurPlaced=false;
    
    Position getAdjacentPosition(Direction dir) const {
        Position pos = dinosaurPos;
        switch(dir) {
            case Direction::UP: pos.y = (pos.y - 1 + height) % height; break;
            case Direction::DOWN: pos.y = (pos.y + 1) % height; break;
            case Direction::LEFT: pos.x = (pos.x - 1 + width) % width; break;
            case Direction::RIGHT: pos.x = (pos.x + 1) % width; break;
        }
        return pos;
    }
    
    Position getJumpPosition(Direction dir, int steps) const {
        Position pos = dinosaurPos;
        switch(dir) {
            case Direction::UP: pos.y = (pos.y - steps + height * steps) % height; break;
            case Direction::DOWN: pos.y = (pos.y + steps) % height; break;
            case Direction::LEFT: pos.x = (pos.x - steps + width * steps) % width; break;
            case Direction::RIGHT: pos.x = (pos.x + steps) % width; break;
        }
        return pos;
    }
    
    vector<Position> getJumpPath(Direction dir, int steps) const {
        vector<Position> path;
        for(int i = 1; i <= steps; i++) {
            Position pos = getJumpPosition(dir, i);
            path.push_back(pos);
        }
        return path;
    }
    
    void updateDisplayGrid() {
        grid = baseGrid;
        if(dinosaurPlaced) {
            grid[dinosaurPos.y][dinosaurPos.x] = '#';
        }
    }
    
    bool isObstacle(char cell) const {
        return cell == '^' || cell == '&' || cell == '@';
    }
    
    bool isEmpty(char cell) const {
        return cell == '_' || (cell >= 'a' && cell <= 'z');
    }
    
public:
    void initialize(int w, int h) {
        width=w; height=h;
        baseGrid.resize(height, vector<char>(width, '_'));
        grid = baseGrid;
        dinosaurPlaced=false;
    }
    
    bool setDinosaur(int x, int y) {
        if(x<0||x>=width||y<0||y>=height) return false;
        dinosaurPos=Position(x,y);
        dinosaurPlaced=true;
        updateDisplayGrid();
        return true;
    }
    
    bool moveDinosaur(Direction dir) {
        if(!dinosaurPlaced) return false;
        
        Position newPos = getAdjacentPosition(dir);
        char targetCell = baseGrid[newPos.y][newPos.x];
        
        if(isObstacle(targetCell)) {
            cerr<<"Warning: Cannot move onto obstacle"<<endl;
            return true;
        }
        if(targetCell == '%') {
            cerr<<"Error: Fell into a hole!"<<endl;
            return false;
        }
        
        dinosaurPos = newPos;
        updateDisplayGrid();
        return true;
    }
    
    bool jumpDinosaur(Direction dir, int steps) {
        if(!dinosaurPlaced || steps <= 0) return false;
        
        vector<Position> path = getJumpPath(dir, steps);
        Position landingPos = path.back();
        
        // Check path for obstacles (excluding landing position)
        for(int i = 0; i < path.size() - 1; i++) {
            if(isObstacle(baseGrid[path[i].y][path[i].x])) {
                Position stopPos = (i == 0) ? dinosaurPos : path[i-1];
                cerr<<"Warning: Obstacle encountered. Stopped at ("<<stopPos.x<<","<<stopPos.y<<")"<<endl;
                dinosaurPos = stopPos;
                updateDisplayGrid();
                return true;
            }
        }
        
        // Check landing position for hole
        if(baseGrid[landingPos.y][landingPos.x] == '%') {
            cerr<<"Error: Fell into a hole during jump!"<<endl;
            return false;
        }
        
        dinosaurPos = landingPos;
        updateDisplayGrid();
        return true;
    }
    
    bool paintCell(char color) {
        if(!dinosaurPlaced || color < 'a' || color > 'z') return false;
        baseGrid[dinosaurPos.y][dinosaurPos.x] = color;
        updateDisplayGrid();
        return true;
    }
    
    bool digHole(Direction dir) {
        if(!dinosaurPlaced) return false;
        Position targetPos = getAdjacentPosition(dir);
        if(targetPos == dinosaurPos) return false;
        
        char& targetCell = baseGrid[targetPos.y][targetPos.x];
        if(targetCell == '^') {
            targetCell = isEmpty(targetCell) ? '_' : targetCell;
        } else {
            targetCell = '%';
        }
        updateDisplayGrid();
        return true;
    }
    
    bool buildMound(Direction dir) {
        if(!dinosaurPlaced) return false;
        Position targetPos = getAdjacentPosition(dir);
        if(targetPos == dinosaurPos) return false;
        
        char& targetCell = baseGrid[targetPos.y][targetPos.x];
        if(targetCell == '%') {
            targetCell = isEmpty(targetCell) ? '_' : targetCell;
        } else {
            targetCell = '^';
        }
        updateDisplayGrid();
        return true;
    }
    
    // Part 6: Trees and Stones
    bool growTree(Direction dir) {
        if(!dinosaurPlaced) return false;
        Position targetPos = getAdjacentPosition(dir);
        if(targetPos == dinosaurPos) return false;
        
        char& targetCell = baseGrid[targetPos.y][targetPos.x];
        if(isEmpty(targetCell)) {
            targetCell = '&';
            updateDisplayGrid();
            return true;
        }
        return false;
    }
    
    bool cutTree(Direction dir) {
        if(!dinosaurPlaced) return false;
        Position targetPos = getAdjacentPosition(dir);
        
        char& targetCell = baseGrid[targetPos.y][targetPos.x];
        if(targetCell == '&') {
            targetCell = '_';
            updateDisplayGrid();
            return true;
        }
        return false;
    }
    
    bool makeStone(Direction dir) {
        if(!dinosaurPlaced) return false;
        Position targetPos = getAdjacentPosition(dir);
        if(targetPos == dinosaurPos) return false;
        
        char& targetCell = baseGrid[targetPos.y][targetPos.x];
        if(isEmpty(targetCell)) {
            targetCell = '@';
            updateDisplayGrid();
            return true;
        }
        return false;
    }
    
    bool pushStone(Direction dir) {
        if(!dinosaurPlaced) return false;
        
        Position stonePos = getAdjacentPosition(dir);
        if(baseGrid[stonePos.y][stonePos.x] != '@') return false;
        
        // Calculate push direction (opposite of dinosaur position relative to stone)
        Position pushDir;
        if(dir == Direction::UP) pushDir = Position(0, -1);
        else if(dir == Direction::DOWN) pushDir = Position(0, 1);
        else if(dir == Direction::LEFT) pushDir = Position(-1, 0);
        else pushDir = Position(1, 0);
        
        Position newStonePos = stonePos;
        newStonePos.x = (newStonePos.x + pushDir.x + width) % width;
        newStonePos.y = (newStonePos.y + pushDir.y + height) % height;
        
        // Check if target position is free
        char& targetCell = baseGrid[newStonePos.y][newStonePos.x];
        if(isObstacle(targetCell)) return false;
        
        // Handle hole filling
        if(targetCell == '%') {
            targetCell = '_';
        } else if(!isEmpty(targetCell)) {
            return false; // Can only push to empty cells or holes
        }
        
        // Move stone
        targetCell = '@';
        baseGrid[stonePos.y][stonePos.x] = '_';
        updateDisplayGrid();
        return true;
    }
    
    void saveToFile(const string& filename) const {
        ofstream file(filename);
        for(const auto& row:grid){
            for(const auto& cell:row) file<<cell;
            file<<endl;
        }
        file.close();
    }
    
    void display() const {
        for(const auto& row:grid){
            for(const auto& cell:row) cout<<cell<<" ";
            cout<<endl;
        }
    }
};

// Utility functions
bool hasLeadingSpaces(const string& line) {
    return !line.empty() && isspace(line[0]);
}

string removeComment(const string& line) {
    size_t commentPos = line.find("//");
    return commentPos != string::npos ? line.substr(0, commentPos) : line;
}

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t");
    if(start == string::npos) return "";
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

void clearConsole() { system(CLEAR_COMMAND); }
void delay(double seconds) {
    this_thread::sleep_for(chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

void displayState(const GameField& field, const string& command, int step) {
    clearConsole();
    cout << "Step " << step << ": " << command << endl;
    cout << "------------------------" << endl;
    field.display();
    cout << "------------------------" << endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    string inputFile, outputFile;
    double interval = 1.0;
    bool displayEnabled = true;
    bool saveEnabled = true;
    
    if(argc < 3) {
        cerr << "Usage: " << argv[0] << " input.txt output.txt [interval N] [no-display] [no-save]" << endl;
        return 1;
    }
    
    inputFile = argv[1];
    outputFile = argv[2];
    
    for(int i = 3; i < argc; i++) {
        string option = argv[i];
        if(option == "interval" && i + 1 < argc) interval = stod(argv[++i]);
        else if(option == "no-display") displayEnabled = false;
        else if(option == "no-save") saveEnabled = false;
    }
    
    GameField field;
    ifstream file(inputFile);
    if(!file.is_open()){
        cerr << "Error: Cannot open input file" << endl;
        return 1;
    }
    
    string line;
    int lineNumber = 0, step = 0;
    bool sizeDefined=false, startDefined=false;
    
    while(getline(file,line)){
        lineNumber++;
        string cleanLine = trim(removeComment(line));
        if(cleanLine.empty()) continue;
        
        if(hasLeadingSpaces(line)) {
            cerr << "Error at line " << lineNumber << ": Leading spaces not allowed" << endl;
            return 1;
        }
        
        istringstream iss(cleanLine);
        string command; iss >> command;
        string originalCommand = command;
        transform(command.begin(), command.end(), command.begin(), ::toupper);
        
        // Command parsing and execution
        bool success = true;
        
        if(command == "SIZE") {
            if(sizeDefined) { cerr << "Error: SIZE already defined" << endl; return 1; }
            int w, h; if(iss >> w >> h && w>0 && h>0) field.initialize(w, h), sizeDefined=true;
            else { cerr << "Error: Invalid SIZE" << endl; return 1; }
        }
        else if(command == "START") {
            if(!sizeDefined) { cerr << "Error: SIZE first" << endl; return 1; }
            if(startDefined) { cerr << "Error: START already defined" << endl; return 1; }
            int x, y; if(iss >> x >> y && field.setDinosaur(x, y)) startDefined=true;
            else { cerr << "Error: Invalid START" << endl; return 1; }
        }
        else if(!startDefined) { cerr << "Error: START first" << endl; return 1; }
        else if(command == "MOVE") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.moveDinosaur(dir);
        }
        else if(command == "JUMP") {
            string dirStr; int steps;
            if(iss >> dirStr >> steps && steps>0) {
                transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
                Direction dir;
                if(dirStr=="UP") dir=Direction::UP;
                else if(dirStr=="DOWN") dir=Direction::DOWN;
                else if(dirStr=="LEFT") dir=Direction::LEFT;
                else if(dirStr=="RIGHT") dir=Direction::RIGHT;
                else { cerr << "Error: Invalid direction" << endl; return 1; }
                success = field.jumpDinosaur(dir, steps);
            } else { cerr << "Error: Invalid JUMP" << endl; return 1; }
        }
        else if(command == "PAINT") {
            char color; if(iss >> color) success = field.paintCell(color);
            else { cerr << "Error: Invalid PAINT" << endl; return 1; }
        }
        else if(command == "DIG") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.digHole(dir);
        }
        else if(command == "MOUND") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.buildMound(dir);
        }
        // Part 6: New commands
        else if(command == "GROW") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.growTree(dir);
        }
        else if(command == "CUT") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.cutTree(dir);
        }
        else if(command == "MAKE") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.makeStone(dir);
        }
        else if(command == "PUSH") {
            string dirStr; iss >> dirStr;
            transform(dirStr.begin(), dirStr.end(), dirStr.begin(), ::toupper);
            Direction dir;
            if(dirStr=="UP") dir=Direction::UP;
            else if(dirStr=="DOWN") dir=Direction::DOWN;
            else if(dirStr=="LEFT") dir=Direction::LEFT;
            else if(dirStr=="RIGHT") dir=Direction::RIGHT;
            else { cerr << "Error: Invalid direction" << endl; return 1; }
            success = field.pushStone(dir);
        }
        else {
            cerr << "Error: Unknown command: " << command << endl;
            return 1;
        }
        
        if(!success) return 1;
        
        if(displayEnabled) {
            displayState(field, cleanLine, ++step);
            if(interval > 0) delay(interval);
        } else {
            step++;
        }
    }
    
    if(displayEnabled) {
        clearConsole();
        cout << "Final State (" << step << " commands):" << endl;
        cout << "------------------------" << endl;
        field.display();
    }
    
    if(saveEnabled) {
        field.saveToFile(outputFile);
        cout << "Saved to: " << outputFile << endl;
    }
    
    return 0;
}