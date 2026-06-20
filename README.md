# Assignment 2 - Chess++

## Introduction
In Assignment 2 (A2) of the OOP1 design practicals (KU), you will implement a game which is based on [Chess](https://en.wikipedia.org/wiki/Chess). Please take care to follow the assignment description exactly, even if you are familiar with the chess rules, because _Chess++_ changes some important rules and adds many new ones.

This document describes the general principle of the game.

Assignment 2 consists of three parts. We recommend completing the parts in order and finishing one before starting the next, because each part builds on the previous one.

- [Design Document - Description of Approach](description/Design_Document.md)
- [Milestone 1 - Structure, Config Parsing and Printing](description/Milestone_1.md)
- [Milestone 2 - Commands, Game Logic and Special Powers](description/Milestone_2.md)

## Learning Goals
- Classes and Objects
- Inheritance
- const
- Strings and Streams

## Specifications
Some general specifications must be followed for every assignment in this course. A list of these specifications can be
found here: [Assessment of Assignments - Deductions](https://coding-tugraz.notion.site/Assessment-of-Assignments-How-do-you-get-your-points-00d2624846f4491391b389e6a48fa180#0af0c923deff4c4bb8740f5698aca451)

Please follow these specifications to avoid point deductions.

## Grading

Assignment 2 is worth **58 points**.

**Attention:** To receive a positive grade in the OOP1 KU your program must fulfill at least the following **minimum requirements** (among other conditions):
- At least **50% of all test cases** for A2 have to be passed.
- Any public test cases marked with **MINIMUM REQUIREMENT** in the test report have to be passed.

If your program does not meet the minimum requirements by the A2 deadline, you will not be allowed to proceed to A3 and will need to revise A2 until the A3 deadline.

After the A2 deadline, if the minimum requirements are met, your team can decide whether to:

- move on to A3 (in this case, A2 will be graded immediately), or
- continue improving A2 until the A3 deadline (A2 will then be graded after the A3 deadline, and you will not receive points for A3).

> **Note:** Since A3 builds on A2, **we strongly recommend passing at least 70% of all A2 test cases** by the A2 deadline to avoid issues later. Your A2 will only be graded once.

An overview image of the procedure can be found [here](https://www.notion.so/coding-tugraz/Guide-for-the-Practicals-OOP1-1aa30869373d80209e17cb2809cfa5d4?pvs=4#1ad30869373d803687d6de1698e5426c).

Information on how your assignment submission will be assessed can be found here: [Assessment of Assignments](https://coding-tugraz.notion.site/Assessment-of-Assignments-How-do-you-get-your-points-00d2624846f4491391b389e6a48fa180)

An overview of how your total KU grade is calculated can be found here: [Procedure and Grading](https://coding-tugraz.notion.site/Practicals-structure-grading-and-plagiarism-76127221ed43451abc7ffba7852595e3)

To receive points for A2, you must attend an **assignment interview**.

### Points for Effective C++

Some of the points for this assignment can only be achieved if you use certain C++ concepts in your implementation.
Information about this can be found here: [Assessment of Assignments - Effective C++ Points](https://coding-tugraz.notion.site/Assessment-of-Assignments-How-do-you-get-your-points-00d2624846f4491391b389e6a48fa180?pvs=25#a94d582d5e6740859ecd613857bd0a36)


## Submission

**Assignment 2 is group work!** It is submitted in teams of three people. Try to keep the division of labor as even as possible. If a team member does not contribute sufficiently to the project, they may get deductions. Every team member should make their own commits so that we are able to see who implemented what. In the artifacts downloaded from Gitlab, you can find a gitreport that shows your current statistics.

### Design Document (**mandatory**)
- Push to your team's repository **on the `main` branch**
- Deliverables: a pdf of your [Design Document](description/Design_Document.md)
- **Deadline: 02.05.2026, 18:00 (Austrian time)**
- There will be discussion rounds with your tutor and other teams to discuss your design. Participating in a discussion round is **mandatory** to receive a positive grade. **Your team must sign up for a discussion round via the Teachcenter before the deadline**.

### Program (**mandatory**)

- Push to your team's repository **on a branch called `submission`**
- Deliverables: all .cpp/.hpp (and optionally other) files needed to run your program
- **Deadline: 27.05.2026, 18:00 (Austrian time)**
- There will be an **assignment interview** for the **entire team**.

## Assignment Description

### General Notes
The notes given in this section are important for the entire assignment description.

- `\n` should not be printed as two separate characters, instead they represent the newline character.
- Note the leading and trailing spaces in the text fields. You can make them visible by selecting the text field.
- Words in uppercase letters in angle brackets (e.g. `<PLAYER_ID>`) should be replaced by calculated or given data. They are _not_ part of the output itself.
- Additional **whitespaces** in user input should be ignored:
  - at the beginning and at the end
  - between command and parameters
  - between parameters
- User inputs are **case-insensitive**. This means that `quit`, `Quit` and `QUIT` are all valid inputs.
- <code>&nbsp;>&nbsp;</code> (command prompt, always printed with a leading and trailing space) in the output indicates that the program should wait for user input at this point. Execution should only continue after the user submits their input with `ENTER`.
- If the user's input is invalid, the corresponding error message should be printed. After this, the same question is asked again, the command prompt is printed, and the user can input again. This process is repeated **as long as** the input is invalid.
- If `quit` or `EOF` is entered (End of File, not the string "EOF"), the program should terminate with the return value `0`. This must be possible any time the program waits for user input, except where noted otherwise.
- The program must correctly handle arbitrary user input, including incorrect data types (e.g., strings instead of integers) and inputs of varying length.

### The Game
The following is a brief overview of the core elements of the game. A more detailed description can be found in [Milestone 1](./description/Milestone_1.md).

In this chess-based game, two players compete to capture the opponent’s king. Different pieces have various special powers that can greatly affect the outcome of the game. While we aimed to keep certain aspects, such as the board, basic movement rules, and move syntax, close to standard chess, the game differs from traditional chess in several key ways:
- The initial setup is not fixed and can be configured via a file.
- Most pieces have special abilities or movement patterns.
- Items can be picked up from the board and used.
- No new base piece types were added (i.e., only pawns, rooks, knights, bishops, queens, and kings exist), but each type may have multiple variants with different behaviors.

>**Note**: The game will likely not be perfectly balanced, as this is a programming course rather than a game design course. If this were a real game, it would of course undergo playtesting and adjustments to improve balance.

#### Start of the Game
At the beginning of the game, each player can decide how to place their pieces on the board.

#### Playing the Game
Players take alternating turns. On each turn, a player may either make a normal chess move or use one of their pieces’ special abilities or items.

Players have a limited amount of mana, which is automatically refreshed each turn. Using special abilities consumes mana and may therefore not always be possible. In contrast, items do not require mana.

Additionally, certain squares on the board can trigger special effects when a piece lands on them.

#### End of the Game

The goal of the game is to **capture the enemy king**. The game also ends if the maximum number of turns is reached.


### Error Handling

During the game, various errors can occur as a result of invalid user input or illegal game actions. A complete overview of all possible errors can be found [here](./description/Errors.md).

Each error is assigned a rank, which defines its priority. If multiple errors occur while processing a command, only the error with the highest priority (i.e., the lowest rank number) is reported.

### Unicode and Chess Piece Rendering

In this assignment, the chess board is displayed using Unicode characters to represent the different chess pieces. As a result, the board may appear incorrectly or misaligned depending on the font used by your Markdown viewer or terminal.

In particular:

* Some fonts do not properly support Unicode chess symbols.
* This can lead to missing characters, placeholder symbols, or incorrect spacing/alignment in the board display.
* Code blocks in Markdown files are especially prone to this issue, as they often use a fixed default font. (GitLab does not allow customization of fonts used in Markdown code blocks.)

Rendering may also differ depending on the operating system and its default fonts. For example, some Linux distributions may display the chess pieces correctly out of the box, while certain Windows setups may not.

To avoid these problems, we recommend:

* Using a Markdown viewer that supports Unicode characters with an appropriate font.
* Changing your terminal font to one that supports chess symbols, such as **DejaVu Sans Mono**.

If you still experience rendering issues, an alternative is to use the provided virtual machine (VM), which is configured to correctly display the board.

Note that these issues are purely visual and do not affect the correctness of your program.



### Documentation Overview
- [Milestone 1](./description/Milestone_1.md)
  - Definitions & Chess++ Specific Rules
  - Command Line Parameters
  - Config File Parsing
  - Initialization
  - Board Printing
  - Turn & Round Structure
  - Mana
- [Milestone 2](./description/Milestone_2.md)
  - Standard Chess Rules
  - Items
  - Player Turn
  - Commands
- [Pieces](./description/Pieces.md)
  - How to read Piece IDs
  - Detailed description of each piece
  - Power-Up Squares
- [Items](./description/Items.md)
  - Detailed description of each potion and tool
- [Errors](./description/Errors.md)
  - Collection of all errors
