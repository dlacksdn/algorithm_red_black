#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// Red-Black Tree based course registration manager
struct CourseTree {
    enum Color { RED, BLACK };

    struct Key { // 암시적 복사 생성자가 만들어져서 작동한다
        int sid;
        string subject;

        bool operator<(Key const& o) const {
            if (sid != o.sid) 
                return sid < o.sid;

            return subject < o.subject;
        }

        bool operator==(Key const& o) const {
            return sid == o.sid && subject == o.subject;
        }
    };

    struct Node {
        Key key;
        string name;
        int semester;
        string phone;
        long long timestamp;
        Color color;
        Node* left, * right, * parent;

        Node(Key k, string n, int sem, string ph, long long ts, Node* nil)
            : key(k), name(n), semester(sem), phone(ph), timestamp(ts),
            color(RED), left(nil), right(nil), parent(nil) {}
    };

    Node* root, * Nil;

    CourseTree() { // 생성자
        // create sentinel Nil node
        Nil = new Node({ 0,"" }, "", 0, "", 0, nullptr);
        Nil->color = BLACK;
        Nil->left = Nil->right = Nil->parent = Nil;
        root = Nil;
    }

    // left rotation at x
    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;

        if (y->left != Nil) 
            y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == Nil)   
            root = y;
        else                 
            x->parent->right = y;

        y->left = x;
        x->parent = y;
    }

    // right rotation at y
    void rightRotate(Node* y) {
        Node* x = y->left;
        y->left = x->right;

        if (x->right != Nil) 
            x->right->parent = y;

        x->parent = y->parent;

        if (y->parent == Nil)
            root = x;
        else if (y == y->parent->right) 
            y->parent->right = x;
        else                        
            y->parent->left = x;

        x->right = y;
        y->parent = x;
    }

    // restore RBT properties after insert
    void insertFixup(Node* z) { // z는 막 insert된 node
        while (z->parent->color == RED) { // double red 발생
            if (z->parent == z->parent->parent->left) { // 왼쪽 서브트리
                Node* y = z->parent->parent->right; // y는 uncle node

                if (y->color == RED) { // recoloring
                    // Case 1: parent and uncle both RED
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }

                else { // restructuring
                    if (z == z->parent->right) { // 이래 버리면 double rotation이 필요하다
                        // Case 2: z is right child
                        /*
                            G(B)
                           /
                          P(R)
                           \
                            Z(R)
                        */
                        z = z->parent;
                        leftRotate(z);
                        /*
                            G(B)
                           /
                          Z(R)
                         /
                        P(R)
                        */
                    }
                    // Case 3: z is left child
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            }

            else {
                // mirror image of above
                Node* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                }

                else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }

                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }

    // insert or update; returns (depth, wasUpdate)
    pair<int, int> insertOrUpdate(int sid, const string& subject, const string& name, int semester, const string& phone, long long ts) {
        Key k{ sid, subject };
        // check for existing key
        Node* x = root;
        Node* y = Nil;

        while (x != Nil) { // insert될 자리 탐색
            y = x;
            if (k == x->key) { // 중복이 존재
                // update timestamp only
                x->timestamp = ts;

                // compute depth
                int d = 0;
                Node* p = x;

                while (p->parent != Nil) {
                    d++;
                    p = p->parent;
                }

                return { d, 1 };
            }

            if (k < x->key) 
                x = x->left;
            else 
                x = x->right;
        }

        // 중복이 없다는 것
        // create new node z
        Node* z = new Node(k, name, semester, phone, ts, Nil);
        z->parent = y;

        if (y == Nil)   
            root = z;
        else if (z->key < y->key) 
            y->left = z;
        else           
            y->right = z;

        // fix colors and structure
        insertFixup(z);

        // compute depth of new node
        int d = 0;
        Node* p = z;
        while (p->parent != Nil) { 
            d++; 
            p = p->parent; 
        }

        return { d, 0 };
    }

    // search by sid: collect all matching into vector
    void collectBySid(Node* x, int sid, vector<Node*>& v) {
        if (x == Nil) return;

        if (sid < x->key.sid) {
            collectBySid(x->left, sid, v);
        }

        else if (sid > x->key.sid) {
            collectBySid(x->right, sid, v);
        }

        else {
            // sid matches: add and search both subtrees
            v.push_back(x);
            collectBySid(x->left, sid, v);
            collectBySid(x->right, sid, v);
        }
    }

    // full tree traversal: collect nodes whose subject matches, sum depths
    void collectBySubject(Node* x, const string& subject, int depth, vector<Node*>& v, long long& sumDepth) {
        if (x == Nil) return;
        if (x->key.subject == subject) {
            v.push_back(x);
            sumDepth += depth;
        }

        collectBySubject(x->left, subject, depth + 1, v, sumDepth);
        collectBySubject(x->right, subject, depth + 1, v, sumDepth);
    }

    // list all (subject, color) for a student
    vector<pair<string, char>> listStudent(int sid) {
        vector<Node*> tmp;
        collectBySid(root, sid, tmp);

        if (tmp.empty()) 
            return {};

        // sort by subject lex
        sort(tmp.begin(), tmp.end(), [](Node* a, Node* b) { 
            return a->key.subject < b->key.subject; 
        });

        vector<pair<string, char>> out;
        out.reserve(tmp.size()); // 요소 수 많으면 성능 향상
        for (auto* n : tmp)
            out.emplace_back(n->key.subject, n->color == RED ? 'R' : 'B');

        return out;
    }

    // count and sum depths for a subject
    pair<int, long long> countSubject(const string& subj) {
        vector<Node*> tmp;
        long long sumD = 0;

        collectBySubject(root, subj, 0, tmp, sumD);

        return { (int)tmp.size(), sumD };
    }

    // get up to K earliest by timestamp for a subject  
    vector<pair<int, char>> topKByTimestamp(const string& subj, int K) {
        vector<Node*> tmp;
        long long dummy = 0;
        collectBySubject(root, subj, 0, tmp, dummy);

        // sort by timestamp ascending
        sort(tmp.begin(), tmp.end(), [](Node* a, Node* b) { 
            return a->timestamp < b->timestamp; 
        });

        vector<pair<int, char>> out;
        int limit = min(K, (int)tmp.size());
        out.reserve(limit);

        for (int i = 0; i < limit; ++i) {
            Node* n = tmp[i];
            out.emplace_back(n->key.sid, n->color == RED ? 'R' : 'B');
        }

        return out;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    CourseTree tree;
    int Q;
    cin >> Q;

    while (Q--) {
        char C;
        cin >> C;

        if (C == 'I') {
            int sid, sem;
            string subject, name, phone;
            long long ts;

            cin >> sid >> subject >> name >> sem >> phone >> ts;

            auto res = tree.insertOrUpdate(sid, subject, name, sem, phone, ts);
            cout << res.first << " " << res.second << "\n";
        }

        else if (C == 'L') {
            int sid;
            cin >> sid;

            auto lst = tree.listStudent(sid);
            if (lst.empty()) {
                cout << "No records found\n";
            }

            else {
                for (auto& p : lst) 
                    cout << p.first << " " << p.second << " ";
                
                cout << "\n";
            }
        }

        else if (C == 'C') {
            string subject;

            cin >> subject;

            auto c = tree.countSubject(subject);
            cout << c.first << " " << c.second << "\n";
        }

        else if (C == 'M') {
            string subject;
            int K;

            cin >> subject >> K;

            auto topk = tree.topKByTimestamp(subject, K);
            for (auto& p : topk) 
                cout << p.first << " " << p.second << " ";
            
            cout << "\n";
        }
    }

    return 0;
}
