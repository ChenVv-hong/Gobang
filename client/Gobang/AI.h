#ifndef AI_H
#define AI_H

#endif // AI_H


#include<iostream>
#include<string>
#include<memory.h>
#include<set>
#include<deque>
#include<vector>

using namespace std;

class GameTree {
private:
    class Node {
    public:
        int32_t value;
        uint32_t depth;
        Node *father;
        set<Node *> children;
        uint8_t cntX, cntY;
        uint8_t board[15][15]{};

        Node() {
            father = nullptr;
            children.clear();
            value = INT32_MIN;
            depth = cntX = cntY = 0;
            memset(board, 0, sizeof(board));
        }

        Node(Node *node, uint8_t opeX, uint8_t opeY) {
            depth = node->depth + 1;
            value = is_max_node() ? INT32_MIN : INT32_MAX;
            father = node;
            children.clear();
            cntX = opeX;
            cntY = opeY;
            memcpy(board, node->board, sizeof(board));
            board[cntX][cntY] = (depth & 1u) ? 'B' : 'W';
        }

        bool is_max_node() {
            return (depth & 1u) ^ 1u;
        }

        static int32_t evaluate_black(string &s) {
            string patterns[31] = {
                    "B0000", "0B000", "00B00", "000B0", "0000B",
                    "BB000", "0BB00", "00BB0", "000BB", "B0B00", "0B0B0", "00B0B", "B00B0", "0B00B", "B000B",
                    "BBB00", "0BBB0", "00BBB", "BB0B0", "0BB0B", "B0BB0", "0B0BB", "BB00B", "B00BB", "B0B0B",
                    "BBBB0", "BBB0B", "BB0BB", "B0BBB", "0BBBB", "BBBBB",
            };
            int32_t scores[31] = {
                    1, 1, 1, 1, 1,
                    10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                    100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
                    10000, 10000, 10000, 10000, 10000, 1000000,
            };
            for (uint8_t i = 0; i < 31; i++)
                if (s == patterns[i]) return scores[i];
            return 0;
        }

        static int32_t evaluate_white(string &s) {
            string patterns[31] = {
                    "W0000", "0W000", "00W00", "000W0", "0000W",
                    "WW000", "0WW00", "00WW0", "000WW", "W0W00", "0W0W0", "00W0W", "W00W0", "0W00W", "W000W",
                    "WWW00", "0WWW0", "00WWW", "WW0W0", "0WW0W", "W0WW0", "0W0WW", "WW00W", "W00WW", "W0W0W",
                    "WWWW0", "WWW0W", "WW0WW", "W0WWW", "0WWWW", "WWWWW",
            };
            int32_t scores[31] = {
                    1, 1, 1, 1, 1,
                    10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
                    1000, 2000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000,
                    100000, 100000, 100000, 100000, 100000, 10000000,
            };
            for (uint8_t i = 0; i < 31; i++)
                if (s == patterns[i]) return scores[i];
            return 0;
        }

        static string convert(uint8_t pos) {
            if (pos == 0) return "0";
            if (pos == 'B') return "B"; else return "W";
        }

        uint8_t board_identify() {
            for (uint8_t i = 0; i < 15; i++)
                for (uint8_t j = 0; j < 15; j++) {
                    if (j + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i][j + k]);
                        if (s == "BBBBB") return 'B';
                        if (s == "WWWWW") return 'W';
                    }
                    if (i + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j]);
                        if (s == "BBBBB") return 'B';
                        if (s == "WWWWW") return 'W';
                    }
                    if (i + 4 < 15 && j + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j + k]);
                        if (s == "BBBBB") return 'B';
                        if (s == "WWWWW") return 'W';
                    }
                    if (i + 4 < 15 && j - 4 >= 0) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j - k]);
                        if (s == "BBBBB") return 'B';
                        if (s == "WWWWW") return 'W';
                    }
                }
            return 0;
        }

        void evaluate() {
            value = 0;
            for (uint8_t i = 0; i < 15; i++)
                for (uint8_t j = 0; j < 15; j++) {
                    if (j + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i][j + k]);
                        value += evaluate_black(s) - evaluate_white(s);
                    }
                    if (i + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j]);
                        value += evaluate_black(s) - evaluate_white(s);
                    }
                    if (i + 4 < 15 && j + 4 < 15) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j + k]);
                        value += evaluate_black(s) - evaluate_white(s);
                    }
                    if (i + 4 < 15 && j - 4 >= 0) {
                        string s;
                        for (uint8_t k = 0; k < 5; k++) s += convert(board[i + k][j - k]);
                        value += evaluate_black(s) - evaluate_white(s);
                    }
                }
        }

        void print_info() {
            cout << this << " depth=" << depth << " value=" << value << " father=" << father << " children=(";
            for (auto child : children) cout << child << ",";
            cout << ")" << endl;
            for (auto &i : board) {
                cout << "    ";
                for (uint8_t j : i) {
                    if (j == 'B') cout << "○";
                    if (j == 'W') cout << "●";
                    if (j == 0) cout << "┼";
                }
                cout << endl;
            }
        }
    };

    uint8_t expandRadius = 2;
    uint32_t maxDepth = 5;
    Node *nodeRoot = new Node();
    Node *nodeNext = nullptr;
    deque<Node *> openTable;
    deque<Node *> closedTable;

    vector<pair<uint8_t, uint8_t>> get_search_nodes(Node *node) {
        bool hasChess = false, newBoard[15][15];
        memset(newBoard, false, sizeof(newBoard));
        for (uint8_t i = 0; i < 15; i++)
            for (uint8_t j = 0; j < 15; j++) {
                if (node->board[i][j] == 0) continue;
                hasChess = true;
                uint8_t x1 = max(0, i - expandRadius), x2 = min(14, i + expandRadius);
                uint8_t y1 = max(0, j - expandRadius), y2 = min(14, j + expandRadius);
                for (uint8_t x = x1; x <= x2; x++)
                    for (uint8_t y = y1; y <= y2; y++)
                        if (node->board[x][y] == 0) newBoard[x][y] = true;
            }

        vector<pair<uint8_t, uint8_t>> mask;

        if (!hasChess) {
            mask.emplace_back(pair<uint8_t, uint8_t>(7, 7));
        } else {
            for (uint8_t i = 0; i < 15; i++)
                for (uint8_t j = 0; j < 15; j++)
                    if (newBoard[i][j])
                        mask.emplace_back(pair<uint8_t, uint8_t>(i, j));
        }

        return mask;
    }

    uint8_t expand_children_nodes(Node *node) {
        vector<pair<uint8_t, uint8_t>> mask = get_search_nodes(node);
        for (auto pos:mask) {
            Node *n = new Node(node, pos.first, pos.second);
            node->children.insert(n);
            openTable.push_front(n);
        }
        return mask.size();
    }

    static bool is_alpha_beta_cut(Node *node) {
        if (node == nullptr || node->father == nullptr) return false;
        if (node->is_max_node() && node->value > node->father->value) return true;
        if (!node->is_max_node() && node->value < node->father->value) return true;
        return is_alpha_beta_cut(node->father);
    }

    static void update_value_from_node(Node *node) {
        if (node == nullptr) return;
        if (node->children.empty()) {
            update_value_from_node(node->father);
            return;
        }
        if (node->is_max_node()) {
            int32_t cntValue = INT32_MIN;
            for (Node *n : node->children)
                if (n->value != INT32_MAX) cntValue = max(cntValue, n->value);
            if (cntValue > node->value) {
                node->value = cntValue;
                update_value_from_node(node->father);
            }
        } else {
            int32_t cntValue = INT32_MAX;
            for (Node *n : node->children)
                if (n->value != INT32_MIN) cntValue = min(cntValue, n->value);
            if (cntValue < node->value) {
                node->value = cntValue;
                update_value_from_node(node->father);
            }
        }
    }

    void set_next_pos() {
        nodeNext = *nodeRoot->children.begin();
        for (Node *n : nodeRoot->children)
            if (n->value > nodeNext->value) nodeNext = n;
    }

    static void recursive_print(Node *nodeFatherPt) {
        nodeFatherPt->print_info();
        for (Node *nodeChildPt : nodeFatherPt->children) recursive_print(nodeChildPt);
    }

    void debug_print() {
        nodeRoot->print_info();
        for (Node *nodeChild : nodeRoot->children) recursive_print(nodeChild);
        cout << endl;
    }

public:
    GameTree() = default;

    explicit GameTree(uint32_t maxDepth, uint8_t expandRadius) : maxDepth(maxDepth), expandRadius(expandRadius) {
    }

    explicit GameTree(uint32_t maxDepth, uint8_t expandRadius, uint8_t (&board)[15][15]) :
            maxDepth(maxDepth), expandRadius(expandRadius) {
        memcpy(nodeRoot->board, board, sizeof(board));
    }

    uint8_t game() {
        uint8_t result = nodeRoot->board_identify();
        if (result == 'B') return 'B';
        if (result == 'W') return 'W';

        openTable.push_back(nodeRoot);
        while (!openTable.empty()) {
            Node *node = openTable.front();
            openTable.pop_front();
            closedTable.push_back(node);
            if (is_alpha_beta_cut(node->father)) continue;
            if (node->depth < maxDepth) {
                uint8_t numExpand = expand_children_nodes(node);
                if (numExpand != 0) continue;
            }
            node->evaluate();
            update_value_from_node(node);
        }

        set_next_pos();
        return 0;
    }
    //下一个落子
    pair<uint8_t, uint8_t> get_next_pos() {
        if (nodeNext == nullptr)
            return pair<uint8_t, uint8_t>(255, 255);
        else
            return pair<uint8_t, uint8_t>(nodeNext->cntX, nodeNext->cntY);
    }

    void show_next_pos() {
        if (nodeNext == nullptr)
            cout << "(255, 255)" << endl;
        else
            cout << "(" << (uint32_t) nodeNext->cntX << "," << (uint32_t) nodeNext->cntY << ")" << endl;
    }

    void show_board(bool reverse) {
        if (nodeNext == nullptr) nodeNext = nodeRoot;
        uint8_t row = 0;
        cout << "   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4" << endl;
        for (uint8_t i = 0; i < 15; i++) {
            if (row < 10) cout << " ";
            cout << uint32_t(row++) << " ";
            for (uint8_t j = 0; j < 15; j++) {
                if (j != 0) cout << "─";
                if (nodeNext->board[i][j] == 'B') {
                    if (reverse) cout << "●"; else cout << "○";
                    continue;
                }
                if (nodeNext->board[i][j] == 'W') {
                    if (reverse) cout << "○"; else cout << "●";
                    continue;
                }
                if (i == 0 && j == 0) {
                    cout << "┌";
                    continue;
                }
                if (i == 0 && j == 14) {
                    cout << "┐";
                    continue;
                }
                if (i == 14 && j == 0) {
                    cout << "└";
                    continue;
                }
                if (i == 14 && j == 14) {
                    cout << "┘";
                    continue;
                }
                if (i == 0) {
                    cout << "┬";
                    continue;
                }
                if (i == 14) {
                    cout << "┴";
                    continue;
                }
                if (j == 0) {
                    cout << "├";
                    continue;
                }
                if (j == 14) {
                    cout << "┤";
                    continue;
                }
                cout << "┼";
            }
            cout << endl;
        }
        cout << endl;
    }
};


class AI{
public:
    AI(){
        memset(board, 0 , sizeof(board));
    }
    void clear(){
        memset(board, 0 , sizeof(board));
    }
    uint8_t board[15][15]{};
    GameTree gt;
};

