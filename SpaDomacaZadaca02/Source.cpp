#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <ctime>
#include <Windows.h>

using namespace std;
using namespace sf;

const int row_limit = 20;
const int column_limit = 40;
const int cell_size = 20;
const int moveInterval = 500;

void displayInitialText(RenderWindow& window) {
    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cout << "Error loading font" << endl;
        return;
    }

    Text text;
    text.setFont(font);
    text.setString("Press Enter to start the simulation...");
    text.setCharacterSize(24);
    text.setFillColor(Color::Red);
    text.setPosition(10, 10);

    window.draw(text);
    window.display();

    // Start upon pressing [ENTER]
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                return;
            }
            if (event.type == Event::Closed) {
                window.close();
                return;
            }
        }
    }
}

void getInputValue(int& value, const string& prompt, int lowerLimit, int upperLimit) {
    while (true) {
        try {
            cout << prompt << endl;
            cin >> value;

            // Check if the value is within the limits
            if (value >= lowerLimit && value <= upperLimit) {
                break;  // Value is valid, exit the loop
            }
            else {
                throw out_of_range("Value is out of range.");  // Throw an exception
            }
        }
        catch (const exception& e) {
            cout << "Invalid input. Please enter a value between " << lowerLimit << " and " << upperLimit << "." << endl;
            cin.clear();   // Clear any error flags
            cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Ignore the remaining input
        }
    }
}

void generateWalls(vector<pair<int, int>>& walls, int startRow, int startColumn, int endRow, int endColumn, int numWalls) {
    srand(time(nullptr));

    for (int i = 0; i < numWalls; i++) {
        int row, startCol, length, direction;
        bool isOverlapping = false;

        do {
            isOverlapping = false;
            row = rand() % row_limit;
            startCol = rand() % (column_limit - 5);  // Start column for the wall
            length = rand() % 5 + 1;  // Length of the wall (1 to 5)
            direction = rand() % 2;  // Direction of the wall (0: horizontal, 1: vertical)

            if (direction == 0) {
                for (int j = startCol; j < startCol + length; j++) {
                    if (row == startRow && j >= startColumn && j <= startColumn + length) {
                        isOverlapping = true;
                        break;
                    }
                    if (row == endRow && j >= endColumn && j <= endColumn + length) {
                        isOverlapping = true;
                        break;
                    }
                    if (find(walls.begin(), walls.end(), make_pair(row, j)) != walls.end()) {
                        isOverlapping = true;
                        break;
                    }
                }
            }
            else {
                for (int j = row; j < row + length; j++) {
                    if ((j >= startRow && j <= startRow + length) && (startCol == startColumn)) {
                        isOverlapping = true;
                        break;
                    }
                    if ((j >= endRow && j <= endRow + length) && (startCol == endColumn)) {
                        isOverlapping = true;
                        break;
                    }
                    if (find(walls.begin(), walls.end(), make_pair(j, startCol)) != walls.end()) {
                        isOverlapping = true;
                        break;
                    }
                }
            }
        } while (isOverlapping);

        if (direction == 0) {
            for (int j = startCol; j < startCol + length; j++) {
                walls.push_back(make_pair(row, j));
            }
        }
        else {
            for (int j = row; j < row + length; j++) {
                walls.push_back(make_pair(j, startCol));
            }
        }
    }
}

void drawGrid(RenderWindow& window, int startRow, int startColumn, int endRow, int endColumn, int currentRow, int currentColumn, vector<pair<int, int>>& walls) {
    window.clear();

    RectangleShape cell(Vector2f(cell_size, cell_size));
    cell.setOutlineThickness(1.f);
    cell.setOutlineColor(Color(100.f, 100.f, 100.f));

    for (int i = 0; i < row_limit; i++) {
        for (int j = 0; j < column_limit; j++) {
            cell.setPosition(j * cell_size, i * cell_size);

            if (i == startRow && j == startColumn) {
                cell.setFillColor(Color::Green);
            }
            else if (i == endRow && j == endColumn) {
                cell.setFillColor(Color::Red);
            }
            else if (find(walls.begin(), walls.end(), make_pair(i, j)) != walls.end()) {
                cell.setFillColor(Color::Blue);
            }
            else if (i == currentRow && j == currentColumn) {
                cell.setFillColor(Color::Yellow);
            }
            else {
                cell.setFillColor(Color(140.f, 140.f, 140.f));
            }

            window.draw(cell);
        }
    }

    window.display();
}

void animateWalls(vector<pair<int, int>>& walls, int startRow, int startColumn, int endRow, int endColumn, int currentRow, int currentColumn, RenderWindow& window) {
    for (int i = 0; i < walls.size(); i++) {
        // Check if the yellow cell is within 1 cell of the wall
        int rowDistance = abs(walls[i].first - currentRow);
        int colDistance = abs(walls[i].second - currentColumn);

        if (rowDistance <= 1 && colDistance <= 1) {
            // Yellow cell is within 1 cell of the wall, so don't move the wall
            continue;
        }

        int direction = rand() % 2;  // Direction of movement for the wall (0: horizontal, 1: vertical)

        if (direction == 0) {
            walls[i].second += rand() % 3 - 1;  // Move the wall horizontally by -1, 0, or 1
        }
        else {
            walls[i].first += rand() % 3 - 1;  // Move the wall vertically by -1, 0, or 1
        }
    }

    drawGrid(window, startRow, startColumn, endRow, endColumn, currentRow, currentColumn, walls);
}

void moveYellowCell(int& startRow, int& startColumn, int& currentRow, int& currentColumn, int endRow, int endColumn, vector<pair<int, int>>& walls, RenderWindow& window) {
    // Calculate the horizontal and vertical distances to the target
    int distanceRow = endRow - currentRow;
    int distanceColumn = endColumn - currentColumn;

    // Determine the direction of movement based on the distances
    int moveRow = (distanceRow > 0) - (distanceRow < 0);
    int moveColumn = (distanceColumn > 0) - (distanceColumn < 0);

    // Check if moving diagonally would encounter a wall or go out of bounds
    bool canMoveDiagonally = currentRow + moveRow >= 0 && currentRow + moveRow < row_limit&& currentColumn + moveColumn >= 0 && currentColumn + moveColumn < column_limit&& find(walls.begin(), walls.end(), make_pair(currentRow + moveRow, currentColumn + moveColumn)) == walls.end(); // Don't even try to understand this :skull:

    // Choose the direction of movement based on available paths and distances
    if (canMoveDiagonally) {
        currentRow += moveRow;
        currentColumn += moveColumn;
//        PlaySound(TEXT("SystemDefault"), nullptr, SND_ALIAS | SND_ASYNC);  This is if you want to freak out someone with random System sounds

    }
    else if (currentRow + moveRow >= 0 && currentRow + moveRow < row_limit && find(walls.begin(), walls.end(), make_pair(currentRow + moveRow, currentColumn)) == walls.end()) {
        // Move vertically if there are no obstacles
        currentRow += moveRow;
    }
    else if (currentColumn + moveColumn >= 0 && currentColumn + moveColumn < column_limit && find(walls.begin(), walls.end(), make_pair(currentRow, currentColumn + moveColumn)) == walls.end()) {
        // Move horizontally if there are no obstacles
        currentColumn += moveColumn;
    }

    drawGrid(window, startRow, startColumn, endRow, endColumn, currentRow, currentColumn, walls);
}

void findBestRoute(int row_limit, int column_limit, int startRow, int startColumn, int endRow, int endColumn, vector<pair<int, int>>& walls, RenderWindow& window) {
    window.setFramerateLimit(5);  // Limit the frame rate for smooth animation

    int currentRow = startRow;
    int currentColumn = startColumn;
    int timer = 0;
    bool isStuck = false;

    while (currentRow != endRow || currentColumn != endColumn) {
        moveYellowCell(startRow, startColumn, currentRow, currentColumn, endRow, endColumn, walls, window);

        if (currentRow == endRow && currentColumn == endColumn) {
            break;  // Reached the end point
        }

        animateWalls(walls, startRow, startColumn, endRow, endColumn, currentRow, currentColumn, window);

        if (currentRow == startRow && currentColumn == startColumn) {
            // The yellow cell is stuck at the start point
            if (isStuck) {
                // The yellow cell has been stuck for more than 500 milliseconds
                cout << "The yellow cell is stuck. Moving it to a random location." << endl;
                currentRow = rand() % row_limit;
                currentColumn = rand() % column_limit;
                drawGrid(window, startRow, startColumn, endRow, endColumn, currentRow, currentColumn, walls);
                isStuck = false;
                timer = 0;
            }
            else {
                // The yellow cell just got stuck
                isStuck = true;
                timer = clock();  // Start the timer
            }
        }
        else {
            // The yellow cell is not stuck
            isStuck = false;
            timer = 0;
        }

        if (isStuck && timer > 500) {
            // The yellow cell has been stuck for more than 500 milliseconds
            cout << "The yellow cell is stuck. Moving it to a random location." << endl;
            currentRow = rand() % row_limit;
            currentColumn = rand() % column_limit;
            drawGrid(window, startRow, startColumn, endRow, endColumn, currentRow, currentColumn, walls);
            isStuck = false;
            timer = 0;
        }

        // Delay for 200 milliseconds to simulate animation
        sleep(milliseconds(200));
        timer += 200;
    }
}


int main() {
    int startRow, startColumn, endRow, endColumn;

    cout << "Starting point - Green" << endl << "End point - Red" << endl << "Pathfinding cell - Yellow" << endl << "Walls - Blue" << endl << endl;

    getInputValue(startRow, "Enter the row for Point A:", 0, row_limit - 1);
    getInputValue(startColumn, "Enter the column for Point A:", 0, column_limit - 1);
    getInputValue(endRow, "Enter the row for Point B:", 0, row_limit - 1);
    getInputValue(endColumn, "Enter the column for Point B:", 0, column_limit - 1);

    vector<pair<int, int>> walls;
    int numWalls = 15;

    RenderWindow window(VideoMode(800, 400), "Pathfinding");
    HWND hwnd = window.getSystemHandle();
    // Bring the window to the foreground
    SetForegroundWindow(hwnd);

    generateWalls(walls, startRow, startColumn, endRow, endColumn, numWalls);

    displayInitialText(window);

    findBestRoute(row_limit, column_limit, startRow, startColumn, endRow, endColumn, walls, window);

    return 0;
}
