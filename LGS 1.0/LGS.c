#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX 100

// ===== STRUKTURY =====
struct UserData {
    char Name[32];
    char Password[32];
    char Role[20];
    int ID;
};

struct Tournament {
    int id;
    char name[50];
    char game[50];
    char organizer[32];
    int day, month, year;
    int hour, minute;
};

// ===== PLIKI =====
FILE *FileName;
FILE *FilePassword;
FILE *FileRole;
FILE *FileTournament;
FILE *FileRegistration;

// ===== SPRAWDZENIE LOGINU =====
int isUsernameTaken(char name[]) {
    char buffer[32];
    FileName = fopen("Name.txt", "r");
    if (!FileName) return 0;

    while (fgets(buffer, 32, FileName)) {
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(name, buffer) == 0) {
            fclose(FileName);
            return 1;
        }
    }

    fclose(FileName);
    return 0;
}

// ===== POBIERANIE ROLI =====
void getRole(char username[], char role[]) {
    char user[32], r[20];

    FileRole = fopen("Roles.txt", "r");
    if (!FileRole) {
        strcpy(role, "gracz");
        return;
    }

    while (fscanf(FileRole, "%s %s", user, r) != EOF) {
        if (strcmp(user, username) == 0) {
            strcpy(role, r);
            fclose(FileRole);
            return;
        }
    }

    fclose(FileRole);
    strcpy(role, "gracz");
}

// ===== REJESTRACJA =====
void registerUser(struct UserData *Data) {
    printf("Login: ");
    scanf("%s", Data->Name);

    if (isUsernameTaken(Data->Name)) {
        printf("Nazwa juz istnieje!\n");
        return;
    }

    printf("Haslo: ");
    scanf("%s", Data->Password);

    FileName = fopen("Name.txt", "a");
    fprintf(FileName, "%s\n", Data->Name);
    fclose(FileName);

    FilePassword = fopen("Password.txt", "a");
    fprintf(FilePassword, "%s\n", Data->Password);
    fclose(FilePassword);

    FileRole = fopen("Roles.txt", "a");
    fprintf(FileRole, "%s gracz\n", Data->Name);
    fclose(FileRole);

    printf("Zarejestrowano!\n");
}

// ===== LOGOWANIE =====
int login(struct UserData *Data) {
    char buffer[32];
    int i = 0;

    printf("Login: ");
    scanf("%s", Data->Name);

    printf("Haslo: ");
    scanf("%s", Data->Password);

    FileName = fopen("Name.txt", "r");

    while (fgets(buffer, 32, FileName)) {
        buffer[strcspn(buffer, "\n")] = 0;
        i++;

        if (strcmp(Data->Name, buffer) == 0) {
            Data->ID = i;
            break;
        }
    }

    fclose(FileName);

    if (Data->ID == 0) {
        printf("Bledne dane\n");
        return 0;
    }

    FilePassword = fopen("Password.txt", "r");

    for (i = 0; i < Data->ID; i++)
        fgets(buffer, 32, FilePassword);

    fclose(FilePassword);

    buffer[strcspn(buffer, "\n")] = 0;

    if (strcmp(Data->Password, buffer) != 0) {
        printf("Bledne dane\n");
        Data->ID = 0;
        return 0;
    }

    getRole(Data->Name, Data->Role);
    return 1;
}

// ===== ZMIANA ROLI =====
void changeRole() {
    char user[32], role[20];
    char u[32], r[20];
    FILE *temp;

    printf("Podaj login: ");
    scanf("%s", user);

    printf("Nowa rola (admin/organizator/gracz): ");
    scanf("%s", role);

    FileRole = fopen("Roles.txt", "r");
    temp = fopen("temp.txt", "w");

    while (fscanf(FileRole, "%s %s", u, r) != EOF) {
        if (strcmp(u, user) == 0)
            fprintf(temp, "%s %s\n", u, role);
        else
            fprintf(temp, "%s %s\n", u, r);
    }

    fclose(FileRole);
    fclose(temp);

    remove("Roles.txt");
    rename("temp.txt", "Roles.txt");

    printf("Zmieniono role!\n");
}

// ===== ID TURNIEJU =====
int getNextTournamentID() {
    struct Tournament t;
    int lastID = 0;

    FileTournament = fopen("tournaments.txt", "r");
    if (!FileTournament) return 1;

    while (fscanf(FileTournament, "%d %s %s %s %d %d %d %d %d",
        &t.id, t.name, t.game, t.organizer,
        &t.day, &t.month, &t.year,
        &t.hour, &t.minute) != EOF) {
        lastID = t.id;
    }

    fclose(FileTournament);
    return lastID + 1;
}

// ===== DODAWANIE TURNIEJU =====
void addTournament(struct UserData Data) {
    struct Tournament t;

    t.id = getNextTournamentID();

    printf("Nazwa: ");
    scanf("%s", t.name);

    printf("Gra: ");
    scanf("%s", t.game);

    strcpy(t.organizer, Data.Name);

    printf("Data (dzien miesiac rok): ");
    scanf("%d %d %d", &t.day, &t.month, &t.year);

    printf("Godzina i minuty: ");
    scanf("%d %d", &t.hour, &t.minute);

    FileTournament = fopen("tournaments.txt", "a");

    fprintf(FileTournament, "%d %s %s %s %d %d %d %d %d\n",
        t.id, t.name, t.game, t.organizer,
        t.day, t.month, t.year,
        t.hour, t.minute);

    fclose(FileTournament);

    printf("Dodano turniej ID: %d\n", t.id);
}

// ===== LICZENIE GRACZY =====
int countPlayers(int tid) {
    int id, count = 0;
    char user[32];

    FileRegistration = fopen("registrations.txt", "r");
    if (!FileRegistration) return 0;

    while (fscanf(FileRegistration, "%d %s", &id, user) != EOF)
        if (id == tid) count++;

    fclose(FileRegistration);
    return count;
}

// ===== SORTOWANIE =====
int compare(struct Tournament a, struct Tournament b) {
    if (a.year != b.year) return a.year - b.year;
    if (a.month != b.month) return a.month - b.month;
    if (a.day != b.day) return a.day - b.day;
    if (a.hour != b.hour) return a.hour - b.hour;
    return a.minute - b.minute;
}

// ===== KALENDARZ =====
void showCalendar() {
    struct Tournament t[MAX];
    int count = 0, i, j;
    int month, year;

    printf("Podaj miesiac i rok: ");
    scanf("%d %d", &month, &year);

    FileTournament = fopen("tournaments.txt", "r");
    if (!FileTournament) {
        printf("Brak turniejow!\n");
        return;
    }

    while (fscanf(FileTournament, "%d %s %s %s %d %d %d %d %d",
        &t[count].id, t[count].name, t[count].game, t[count].organizer,
        &t[count].day, &t[count].month, &t[count].year,
        &t[count].hour, &t[count].minute) != EOF) {

        if (t[count].month == month && t[count].year == year)
            count++;
    }

    fclose(FileTournament);

    if (count == 0) {
        printf("Brak turniejow\n");
        return;
    }

    for (i = 0; i < count - 1; i++)
        for (j = 0; j < count - i - 1; j++)
            if (compare(t[j], t[j + 1]) > 0) {
                struct Tournament temp = t[j];
                t[j] = t[j + 1];
                t[j + 1] = temp;
            }

    printf("\n--- KALENDARZ ---\n");

    for (i = 0; i < count; i++)
        printf("ID:%d | %s | %s | %02d-%02d-%d %02d:%02d | Gracze:%d\n",
            t[i].id, t[i].name, t[i].game,
            t[i].day, t[i].month, t[i].year,
            t[i].hour, t[i].minute,
            countPlayers(t[i].id));
}

// ===== SZCZEGӣY =====
void showDetails() {
    int id, found = 0, tid;
    struct Tournament t;
    char user[32];

    printf("Podaj ID: ");
    scanf("%d", &id);

    FileTournament = fopen("tournaments.txt", "r");

    while (fscanf(FileTournament, "%d %s %s %s %d %d %d %d %d",
        &t.id, t.name, t.game, t.organizer,
        &t.day, &t.month, &t.year,
        &t.hour, &t.minute) != EOF) {

        if (t.id == id) {
            found = 1;
            printf("\n%s (%s)\nOrganizator: %s\n",
                t.name, t.game, t.organizer);

            printf("Data: %02d-%02d-%d %02d:%02d\n",
                t.day, t.month, t.year,
                t.hour, t.minute);
        }
    }

    fclose(FileTournament);

    if (!found) {
        printf("Nie znaleziono!\n");
        return;
    }

    printf("Gracze:\n");

    FileRegistration = fopen("registrations.txt", "r");

    if (!FileRegistration) return;

    while (fscanf(FileRegistration, "%d %s", &tid, user) != EOF)
        if (tid == id)
            printf("- %s\n", user);

    fclose(FileRegistration);
}

// ===== ZAPIS =====
void registerToTournament(struct UserData Data) {
    int id, tid;
    char user[32];

    printf("Podaj ID: ");
    scanf("%d", &id);

    FileRegistration = fopen("registrations.txt", "r");

    if (FileRegistration) {
        while (fscanf(FileRegistration, "%d %s", &tid, user) != EOF) {
            if (tid == id && strcmp(user, Data.Name) == 0) {
                printf("Juz zapisany!\n");
                fclose(FileRegistration);
                return;
            }
        }
        fclose(FileRegistration);
    }

    FileRegistration = fopen("registrations.txt", "a");
    fprintf(FileRegistration, "%d %s\n", id, Data.Name);
    fclose(FileRegistration);

    printf("Zapisano!\n");
}

// ===== MAIN =====
int main() {
    struct UserData Data = {"", "", "", 0};
    int input;

    while (1) {
        if (Data.ID == 0) {
            printf("\n1.Rejestracja\n2.Logowanie\n0.Wyjscie\n");
            scanf("%d", &input);

            if (input == 1) registerUser(&Data);
            else if (input == 2 && login(&Data))
                printf("Zalogowano jako %s (%s)\n", Data.Name, Data.Role);
            else if (input == 0) break;

        } else {
            printf("\nZalogowany jako: %s (%s)\n", Data.Name, Data.Role);

            if (strcmp(Data.Role, "admin") == 0) {
                printf("1.Dodaj turniej\n2.Kalendarz\n3.Zapis\n4.Szczegoly\n5.Zmien role\n6.Logout\n");
                scanf("%d", &input);

                if (input == 1) addTournament(Data);
                if (input == 2) showCalendar();
                if (input == 3) registerToTournament(Data);
                if (input == 4) showDetails();
                if (input == 5) changeRole();
                if (input == 6) Data.ID = 0;
            }

            else if (strcmp(Data.Role, "organizator") == 0) {
                printf("1.Dodaj turniej\n2.Kalendarz\n3.Szczegoly\n4.Logout\n");
                scanf("%d", &input);

                if (input == 1) addTournament(Data);
                if (input == 2) showCalendar();
                if (input == 3) showDetails();
                if (input == 4) Data.ID = 0;
            }

            else {
                printf("1.Kalendarz\n2.Zapis\n3.Szczegoly\n4.Logout\n");
                scanf("%d", &input);

                if (input == 1) showCalendar();
                if (input == 2) registerToTournament(Data);
                if (input == 3) showDetails();
                if (input == 4) Data.ID = 0;
            }
        }
    }

    return 0;
}
