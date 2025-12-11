#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROOM_FILE "rooms.txt"
#define CRE_FILE  "credentials.txt"
#define MAX_STACK 100

char currentUser[50];
char currentRole[20];

/* ---------- STACK STRUCTURE FOR OPERATION SIMULATION ---------- */

typedef struct {
    char user[50];   // username who did the operation
    char role[20];   // role: manager / reception / guest
    char op[20];     // operation name: ADD_ROOM, BOOK, CHECKOUT, SEARCH, DISPLAY
    int roomNo;      // room number involved (-1 if not specific)
    char info[50];   // extra info like guest name, "-", etc.
} Operation;

Operation opStack[MAX_STACK];
int top = -1;

int isStackFull() {
    return top == MAX_STACK - 1;
}

int isStackEmpty() {
    return top == -1;
}

void pushOp(const char *op, int roomNo, const char *info) {
    if (isStackFull()) {
        printf("Operation stack is FULL! Cannot log more operations.\n");
        return;
    }
    top++;
    strcpy(opStack[top].user, currentUser);
    strcpy(opStack[top].role, currentRole);
    strcpy(opStack[top].op, op);
    opStack[top].roomNo = roomNo;
    if (info)
        strcpy(opStack[top].info, info);
    else
        strcpy(opStack[top].info, "-");
}

Operation popOp() {
    Operation dummy;
    strcpy(dummy.user, "NONE");
    strcpy(dummy.role, "NONE");
    strcpy(dummy.op, "NONE");
    dummy.roomNo = -1;
    strcpy(dummy.info, "-");

    if (isStackEmpty()) {
        printf("Operation stack is EMPTY! Nothing to undo.\n");
        return dummy;
    }

    Operation last = opStack[top];
    top--;
    return last;
}

void showStack() {
    if (isStackEmpty()) {
        printf("\nNo operations logged yet. Stack is empty.\n");
        return;
    }

    printf("\n--- OPERATION STACK (TOP to BOTTOM) ---\n");
    for (int i = top; i >= 0; i--) {
        printf("Index %d -> User: %s | Role: %s | Operation: %s | Room: %d | Info: %s\n",
               i, opStack[i].user, opStack[i].role,
               opStack[i].op, opStack[i].roomNo, opStack[i].info);
    }
    printf("---------------------------------------\n");
}

void undoLastOperation() {
    if (isStackEmpty()) {
        printf("\nNo operations to undo.\n");
        return;
    }

    Operation last = popOp();
    printf("\nSimulating UNDO of last operation:\n");
    printf("User: %s | Role: %s | Operation: %s | Room: %d | Info: %s\n",
           last.user, last.role, last.op, last.roomNo, last.info);
    printf("(This is only a simulation. Room file is NOT actually changed.)\n");
}

/* ---------------------- LOGIN FUNCTION ------------------------ */

int login() {
    char u[50], p[50], r[20];
    char inUser[50], inPass[50];

    printf("USERNAME: ");
    scanf("%s", inUser);
    printf("PASSWORD: ");
    scanf("%s", inPass);

    FILE *fp = fopen(CRE_FILE, "r");
    if (!fp) {
        printf("Credential file missing!\n");
        return 0;
    }

    while (fscanf(fp, "%s %s %s", u, p, r) == 3) {
        if (strcmp(inUser, u) == 0 && strcmp(inPass, p) == 0) {
            strcpy(currentUser, u);
            strcpy(currentRole, r);  // manager / reception / guest
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

/* -------------------- HOTEL ROOM OPERATIONS ------------------- */
/*
   rooms.txt structure (one room per line):

   roomNo roomType status guestName

   Example:
   101 Single Empty -
   102 Deluxe Booked Rahul

   roomType, status, guestName are single words (no spaces)
*/

void addRoom() {
    int roomNo;
    char roomType[20];

    printf("Enter new Room Number: ");
    scanf("%d", &roomNo);
    printf("Enter Room Type (Single/Double/Deluxe): ");
    scanf("%s", roomType);

    FILE *fp = fopen(ROOM_FILE, "a");
    if (!fp) {
        printf("Error opening room file!\n");
        return;
    }

    // New room is initially Empty, guestName is "-"
    fprintf(fp, "%d %s Empty -\n", roomNo, roomType);
    fclose(fp);

    printf("Room %d added successfully!\n", roomNo);
    pushOp("ADD_ROOM", roomNo, roomType);
}

void displayRooms() {
    FILE *fp = fopen(ROOM_FILE, "r");
    if (!fp) {
        printf("No room file found!\n");
        return;
    }

    int roomNo;
    char roomType[20];
    char status[20];
    char guestName[50];

    printf("\nRoom\tType\tStatus\tGuest\n");
    printf("----\t----\t------\t-----\n");

    while (fscanf(fp, "%d %s %s %s", &roomNo, roomType, status, guestName) == 4) {
        printf("%d\t%s\t%s\t%s\n", roomNo, roomType, status, guestName);
    }

    fclose(fp);

    // Log display operation (no specific room)
    pushOp("DISPLAY", -1, "-");
}

void searchRoom() {
    int find;
    printf("Enter Room Number to search: ");
    scanf("%d", &find);

    FILE *fp = fopen(ROOM_FILE, "r");
    if (!fp) {
        printf("No room file found!\n");
        return;
    }

    int roomNo;
    char roomType[20];
    char status[20];
    char guestName[50];
    int found = 0;

    while (fscanf(fp, "%d %s %s %s", &roomNo, roomType, status, guestName) == 4) {
        if (roomNo == find) {
            printf("Room Found -> Room: %d | Type: %s | Status: %s | Guest: %s\n",
                   roomNo, roomType, status, guestName);
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found)
        printf("Room %d not found!\n", find);

    pushOp("SEARCH", find, "-");
}

void bookRoom() {
    int bookRoomNo;
    char guestName[50];

    printf("Enter Room Number to book: ");
    scanf("%d", &bookRoomNo);
    printf("Enter Guest Name (single word): ");
    scanf("%s", guestName);

    FILE *fp = fopen(ROOM_FILE, "r");
    if (!fp) {
        printf("No room file found!\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Temp file error!\n");
        fclose(fp);
        return;
    }

    int roomNo;
    char roomType[20];
    char status[20];
    char gName[50];
    int found = 0;

    while (fscanf(fp, "%d %s %s %s", &roomNo, roomType, status, gName) == 4) {
        if (roomNo == bookRoomNo) {
            found = 1;
            if (strcmp(status, "Empty") == 0) {
                // Book it
                fprintf(temp, "%d %s Booked %s\n", roomNo, roomType, guestName);
            } else {
                // Already booked, keep as it is
                printf("Room %d is already %s by %s.\n", roomNo, status, gName);
                fprintf(temp, "%d %s %s %s\n", roomNo, roomType, status, gName);
            }
        } else {
            fprintf(temp, "%d %s %s %s\n", roomNo, roomType, status, gName);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(ROOM_FILE);
    rename("temp.txt", ROOM_FILE);

    if (!found) {
        printf("Room %d not found!\n", bookRoomNo);
    } else {
        printf("Booking attempt completed for Room %d.\n", bookRoomNo);
        pushOp("BOOK", bookRoomNo, guestName);
    }
}

void checkOutRoom() {
    int roomNoCheck;
    printf("Enter Room Number for Check-out: ");
    scanf("%d", &roomNoCheck);

    FILE *fp = fopen(ROOM_FILE, "r");
    if (!fp) {
        printf("No room file found!\n");
        return;
    }

    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Temp file error!\n");
        fclose(fp);
        return;
    }

    int roomNo;
    char roomType[20];
    char status[20];
    char guestName[50];
    int found = 0;
    char oldGuest[50] = "-";

    while (fscanf(fp, "%d %s %s %s", &roomNo, roomType, status, guestName) == 4) {
        if (roomNo == roomNoCheck) {
            found = 1;
            if (strcmp(status, "Booked") == 0) {
                strcpy(oldGuest, guestName);
                fprintf(temp, "%d %s Empty -\n", roomNo, roomType);
            } else {
                printf("Room %d is already %s.\n", roomNo, status);
                fprintf(temp, "%d %s %s %s\n", roomNo, roomType, status, guestName);
            }
        } else {
            fprintf(temp, "%d %s %s %s\n", roomNo, roomType, status, guestName);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(ROOM_FILE);
    rename("temp.txt", ROOM_FILE);

    if (!found) {
        printf("Room %d not found!\n", roomNoCheck);
    } else {
        printf("Check-out attempt completed for Room %d.\n", roomNoCheck);
        pushOp("CHECKOUT", roomNoCheck, oldGuest);
    }
}

/* --------------------------- MENUS ---------------------------- */

void managerMenu() {
    int c;
    while (1) {
        printf("\nMANAGER MENU\n");
        printf("1. Add Room\n");
        printf("2. Display Rooms\n");
        printf("3. Search Room\n");
        printf("4. Book Room\n");
        printf("5. Check-out Room\n");
        printf("6. Show Operation Stack (Simulation)\n");
        printf("7. Undo Last Operation (Simulation)\n");
        printf("8. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &c);

        if (c == 1) addRoom();
        else if (c == 2) displayRooms();
        else if (c == 3) searchRoom();
        else if (c == 4) bookRoom();
        else if (c == 5) checkOutRoom();
        else if (c == 6) showStack();
        else if (c == 7) undoLastOperation();
        else return;
    }
}

void receptionMenu() {
    int c;
    while (1) {
        printf("\nRECEPTION MENU\n");
        printf("1. Display Rooms\n");
        printf("2. Search Room\n");
        printf("3. Book Room\n");
        printf("4. Check-out Room\n");
        printf("5. Show Operation Stack (Simulation)\n");
        printf("6. Undo Last Operation (Simulation)\n");
        printf("7. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &c);

        if (c == 1) displayRooms();
        else if (c == 2) searchRoom();
        else if (c == 3) bookRoom();
        else if (c == 4) checkOutRoom();
        else if (c == 5) showStack();
        else if (c == 6) undoLastOperation();
        else return;
    }
}

void guestMenu() {
    int c;
    while (1) {
        printf("\nGUEST MENU\n");
        printf("1. Display Rooms\n");
        printf("2. Search Room\n");
        printf("3. Show Operation Stack (Simulation)\n");
        printf("4. Logout\n");
        printf("Enter choice: ");
        scanf("%d", &c);

        if (c == 1) displayRooms();
        else if (c == 2) searchRoom();
        else if (c == 3) showStack();
        else return;
    }
}

/* --------------------------- MAIN ----------------------------- */

int main() {
    if (!login()) {
        printf("Invalid login!\n");
        return 0;
    }

    printf("\nWelcome %s! Logged in as ROLE: %s\n", currentUser, currentRole);

    if (strcmp(currentRole, "manager") == 0)
        managerMenu();
    else if (strcmp(currentRole, "reception") == 0)
        receptionMenu();
    else
        guestMenu();

    return 0;
}
