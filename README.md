🔢 Quasigroup Analyzer
Project Description
Quasigroup Analyzer is a C++ program designed to work with finite quasigroups, algebraic structures defined by a binary operation forming a Latin square. The program supports generating quasigroup Cayley tables (via cyclic groups, parametric/affine quasigroups, or sequential replacement graphs), checking for proper and nontrivial subquasigroups, and analyzing their properties. It includes an implementation of an affine quasigroup with the operation $(x \cdot y = \alpha x + \beta f(y) + c \mod n)$. The project is intended for educational and research purposes in algebraic combinatorics.
How to Run
Prerequisites

A C++ compiler (e.g., g++ with C++11 or later).
Standard C++ libraries (<vector>, <unordered_set>, <random>, etc.).
A terminal or command-line interface.

Steps

Clone the Repository:
git clone <repository-url>
cd quasigroup-analyzer


Compile the Code:Use a C++ compiler to build the program. For example, with g++:
g++ -std=c++11 main.cpp -o quasigroup_analyzer


Run the Program:Execute the compiled binary:
./quasigroup_analyzer


Interact with the Program:

Choose input methods (1–5) to generate or load a quasigroup:
1: Read Cayley table from a file.
2: Input Cayley table manually.
3: Generate a cyclic group $((x \cdot y = (x + y) \mod n))$.
4: Generate an affine quasigroup $((x \cdot y = \alpha x + \beta f(y) + c \mod n))$.
5: Generate via sequential replacement graph.
6: Exit.


Perform actions (1–6) to check for subquasigroups or save results.
Follow prompts to input parameters (e.g., order ($n$), coefficients ($\alpha, \beta, c$)).



Example
To generate an affine quasigroup of order 5:

Select option 4.
Enter (n = 5), ($\alpha$ = 2), ($\beta$ = 3), ($c$ = 1) (ensuring ($\gcd(\alpha, n)$ = 1), ($\gcd(\beta, n)$ = 1)).
View the generated Cayley table and check for subquasigroups.

Future Steps and Contributions
The project is open to contributions! Planned enhancements include:

Implementing a finite field ($GF(p^m)$) for constructing affine quasigroups.
Adding Damm’s algorithm for quasigroup generation.
Optimizing the subquasigroup search algorithm (currently ($O(n^4)$)).
Supporting sequence encoding using quasigroup operations (e.g., transformation (E)).

How to Contribute

Fork the repository.
Create a branch for your feature (git checkout -b feature-name).
Commit changes (git commit -m "Add feature-name").
Push to your fork (git push origin feature-name).
Open a pull request with a clear description of your changes.

Feel free to suggest ideas, report bugs, or implement new features via GitHub issues or pull requests. Please ensure code follows the existing style and includes comments for clarity.
