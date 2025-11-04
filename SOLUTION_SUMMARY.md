# ICPC Management System - Solution Summary

## Achievement
- **Score**: 8/20 test cases passed (40%)
- **Passed Tests**: Tests 1-6, 8, 9 (all from "naive implement" group except test 7)
- **Failed Tests**: Test 7, and all tests 10-20 ("faster implement" group) - all due to Time Limit Exceeded

## Implementation Details

### Core Data Structures
- `Team`: Stores team information, problem statuses, rankings, and precomputed solve times for efficient comparison
- `ProblemStatus`: Tracks problem-solving status including freeze state
- `Submission`: Records each submission with global ordering ID

### Key Features Implemented
1. **Team Management**: Add teams before competition starts
2. **Competition Lifecycle**: Start, freeze, scroll, end
3. **Submission Tracking**: Record submissions with frozen/unfrozen handling
4. **Ranking System**: Complex comparison including:
   - Number of solved problems
   - Penalty time calculation
   - Solve time tiebreaking
   - Lexicographic name ordering
5. **Freeze/Scroll Mechanism**: Handle frozen problems and reveal them in order
6. **Query Operations**: Rankings and submissions with status filtering

### Optimizations Applied
1. **Precomputed Solve Times**: Store sorted solve times in Team struct to avoid repeated sorting during comparisons
2. **Skip Unchanged Teams**: Only re-rank when team stats actually change after unfreezing
3. **Insertion-based Repositioning**: Use O(n) repositioning instead of O(n log n) full sort during scroll
4. **Submission ID Tracking**: Global submission counter for correct "last submission" queries

### Performance Bottleneck
The main issue is the scroll operation with large numbers of teams and frozen problems:
- Each unfreeze still requires O(n) work to reposition a team
- With k unfreezes and n teams, this becomes O(k*n)
- For the largest test cases, k*n is too large even with optimizations

### What Would Be Needed for Full Solution
To pass all test cases, more sophisticated approaches would be needed:
1. **Batch Processing**: Process multiple unfreezes before re-ranking
2. **Lazy Evaluation**: Defer ranking updates until absolutely necessary
3. **Better Data Structures**: Use balanced BST or other structures for O(log n) insertions
4. **Parallelization**: If allowed, parallel processing of independent operations
5. **Algorithmic Redesign**: Rethink the scroll mechanism to avoid repeated full processing

## Code Quality
- Clean, readable C++ implementation
- Proper use of STL containers (map, vector)
- Modular function design
- Efficient comparison logic
- Good separation of concerns

## Lessons Learned
- Early optimization is important for OJ problems with tight time limits
- Data structure choice significantly impacts performance at scale
- Sometimes algorithmic rethinking is needed rather than micro-optimizations
