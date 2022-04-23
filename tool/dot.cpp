#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace dot {
class Graph {

private:
  string title;
  string result;

  unordered_map<string, vector<string>> adj;
  unordered_map<string, string> labels;

  string genVertice(string &node) {
    string label = this->labels[node];

    return '"' + node + "\" [shape=record, label=\"{" + label + "}\"];\n";
  }

  string genEdge(string &n1, string &n2) {
    return '"' + n1 + "\" -> \"" + n2 + "\";\n";
  }

public:
  Graph(string title) : title(title) {}

  void node(string node, string label) {
    this->adj[node] = vector<string>();
    this->labels[node] = label;
  }

  void edge(string n1, string n2) { this->adj[n1].push_back(n2); }

  string genDot() {

    if (this->result != "")
      return this->result;

    this->result = "digraph \"" + this->title + "\" {\n";

    for (pair<string, vector<string>> entry : adj) {
      string node = entry.first;
      this->result += this->genVertice(node);

      vector<string> successors = entry.second;
      for (string &succ : successors)
        this->result += this->genEdge(node, succ);
    }

    return this->result + "}";
  }
};
} // namespace dot
