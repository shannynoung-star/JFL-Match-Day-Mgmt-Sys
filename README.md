# JFL-Match-Day-Mgmt-Sys
Jamaican Football League and Match-Day Management System

1. Project overview 
The Kingston Premier Football League (KPFL) requires a group of three to five members to create an object
oriented solution to manage their football competition. The KPFL consists of football teams, players, coaches, match 
officials, stadiums, fixtures, ticket sales, merchandise, and match-day operations.  
The minimum submission should be a functional application with persistent data storage using sequential 
access files, random access files, or both. You may include a graphical user interface (GUI) as the interface for your 
solution; however, it should only complement the functionality of your solution, not replace the required object
oriented design, validation, exception handling, and testing. 


2. Scenario 
The scenario is as follows: In the Parish of Kingston, the Kingston Premier Football League (KPFL) runs a 
data-driven football competition each year. The competition organizers allow all teams to request permission to use a 
stadium for practice before each match. Each Team has a name, parish, registration number, squad size, league status, 
players, and coaching staff. 
Each Stadium has a stadium ID, name, location, seating capacity, field condition, ticket price, and a record of 
the teams assigned to the current fixture. The Football League manages teams, stadiums, fixtures, results, and league 
standings. A League Commissioner approves registrations, inspects match preparations, and coordinates the 
assignment of Match Officials to each match. 
The League Commissioner, Player, Coach, Match Official, and Spectator are all specialised types of Person. 
Each Person has an identification number, name, age, and gender. On match days, spectators may purchase tickets 
and products from the Stadium Shop, including merchandise, food, and beverages. All prices include a 15% general 
consumption tax (GCT).



4. Core functional requirements 
1. Register, update, search for, and display football teams. 
2. Add, update, and remove players and coaches associated with a team. 
3. Register stadiums and record seating capacity and field condition. 
4. Create fixtures and assign teams, stadiums, dates, and times. 
5. Register and assign match officials without scheduling conflicts. 
6. Record match results and calculate league points and standings. 
7. Sell tickets while monitoring stadium capacity. 
8. Manage products and complete stadium-shop transactions. 
9. Calculate subtotal, tax, total payment, and change. 
10. Save and reload system data using files. 
11. Produce at least three useful management reports. 
12. Use exceptions to handle invalid data and prohibited operations.

    

5. Mandatory Business Rules for the competition 
• An unregistered or suspended team cannot be assigned to a fixture. 
• A team cannot participate in two fixtures scheduled for the same day and time. 
• A match official cannot be assigned to conflicting fixtures. 
• Ticket sales cannot exceed the stadium's seating capacity. 
• Match scores and quantities cannot be negative. 
• Players must satisfy the league's stated minimum-age rule. 
• A transaction cannot be completed when payment is insufficient. 
• A fixture cannot be completed until a valid result has been recorded.


9. Weekly requirements 
Week 1 - Requirements analysis and planning and System Design 
• Identify functional and non-functional requirements. 
• Develop at least five use cases. 
• Identify actors, constraints, and business rules. 
• Assign initial group responsibilities. 
• Create a shared repository/workspace and project schedule. 
• Submit a UML class diagram with multiplicities and relationship types. 
• Explain inheritance, composition, and aggregation decisions. 
• Design the menu or graphical interface. 
• Define the persistence approach and validation strategy. 
• Prepare the initial test plan. 
Week 2 - Core implementation 
• Implement constructors, accessors, mutators, and validation. 
• Implement the Person hierarchy, teams, coaches, players, and stadiums. 
• Implement suitable collections and object display methods. 
• Demonstrate a working console prototype and initial tests. 
Week 3 - League operations 
• Implement fixtures, stadium assignment, and official assignment. 
• Record match results and update standings. 
• Apply the points system: win = 3, draw = 1, loss = 0. 
• Implement and demonstrate custom exceptions. 
Week 4 - Match-day services and persistence 
• Implement ticket sales and capacity monitoring. 
• Implement inventory, shop transactions, tax, and receipts. 
• Save and reload data. 
• Implement search facilities and at least three reports. 
Week 5 - Integration, testing and presentation 
• Integrate all modules and correct defects. 
• Test normal, boundary, and invalid cases. 
• Submit code, documentation, contribution log, and reflections. 
• Deliver a 10–15-minute demonstration and answer individual questions in interviews.


11. Final submission package 
• Requirements and design document 
• UML class diagram 
• Complete, organized source code 
• Executable application and setup instructions 
• Test plan with results and evidence 
• User guide 
• Technical report 
• Contribution log 
• Individual reflection from every member 
• Presentation slides and live demonstration


13. Individual accountability 
The submission is assessed as a group product, but individual marks may be adjusted using the following evidence: 
• weekly contribution logs and repository history; 
• peer evaluations; 
• attendance and participation in weekly demonstrations; 
• individual reflections; 
• questions directed to specific students during the final defense; and 
• each member's ability to explain the design and code attributed to them. 
A group member who cannot explain the claimed contribution will not automatically receive the group's full 
implementation mark.


14. Submission and presentation requirements 
• Submit one compressed project folder using the naming convention specified by the lecturer. 
• Include clear instructions for compiling, configuring, and running the application. 
• Do not submit generated build folders unless requested. 
• The demonstration must use prepared sample data but should also accept live input. 
• Every group member must speak and answer at least one technical question. 
• Sources, frameworks, and AI-assisted work must be acknowledged according to course policy.


16. Optional extension features 
Attempt extensions only after you have completed and tested all core requirements. 
• JavaFX or another graphical user interface 
• database-backed persistence 
• player-transfer management 
• disciplinary cards and suspension tracking 
• top-scorer and attendance analytics 
• role-based login and access control 
• exportable PDF or CSV reports 
