
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#define NB_JOUEURMAX 10 // nombre de joueur max
#define NB_ROBOTMAX 10  // nombre de robot max
#define TAILLEMAX 512   // taille max du message

int NB_JOUEUR = 0; // nombre de joueur
int NB_ROBOT = 0;  // nombre de robot

char *message[TAILLEMAX]; // message envoyé au client

void error(char *msg)
{
    perror(msg);
    exit(1);
}
/**
 * @fn void generationCartedeJeu()
 * @brief Génération des cartes de jeu
 */
void generationCartedeJeu();
/**
 * @fn void distributionCarte()
 * @brief Distribution des cartes aux joueurs
 */
void distributionCarte();
/**
 * @fn void choixCarte(char * message)
 * @brief Choix de la carte par le joueur
 * @param message message envoyé par le client
 */
void choixCarte(char *message);

/**
 * @fn void AffichageTable(int numJoueur)
 * @brief Affichage de la table de jeu
 * @param numJoueur numéro du joueur
 */
void AffichageTable(int numJoueur);
/**
 * @fn void traitementMessage(char * buffer, int deroulement)
 * @brief Traitement du message envoyé par le client
 * @param buffer message envoyé par le client
 * @param deroulement numéro du joueur
 */
void traitementMessage(char *buffer, int deroulement);

/**
 * @fb void AffichageTableToutJoueur()
 * @brief Affichage de la table de jeu pour tous les joueurs
 */
void AffichageTableToutJoueur();

/**
 * @fn void tourRobot(int numrobot)
 * @brief Tour des robots
 * @param numrobot numéro du robot
 */
void tourRobot(int numrobot);
/**
 * @fn void choixpositionRobot(int choixcarte,int valeurcarte,int numrobot)
 * @brief Choix de la position de la carte par le robot
 * @param choixcarte numéro de la carte
 * @param valeurcarte valeur de la carte
 * @param numrobot numéro du robot
 */
void choixpositionRobot(int choixcarte, int valeurcarte, int numrobot);
/**
 * @fn void fintour()
 * @brief Fin du tour
 */
void fintour();
/**
 * @fn void resetTable()
 * @brief Remise à zéro de la table de jeu
 */
void resetTable();
/**
 * @struct CarteJeu
 * @brief Structure représentant une carte de jeu.
 *
 * Cette structure contient les informations relatives à une carte de jeu,
 * notamment sa valeur et le nombre de taureaux associés.
 */
struct CarteJeu
{
    int valeur;    // valeur de la carte
    int nbTaureau; // nombre de taureau
};
/**
 * @struct Joueur
 * @brief Représente un joueur avec sa liste de cartes et son score.
 */
struct Joueur
{
    struct CarteJeu ListeCarte[10]; // liste des cartes du joueur
    int score;                      // score du joueur
};

struct CarteJeu ListeCarteJeu[104]; // liste des cartes de jeu

int ListeJoueur[NB_JOUEURMAX];                               // liste socket des joueurs
struct Joueur ListeJoueurStruct[NB_JOUEURMAX + NB_ROBOTMAX]; // liste des joueurs

const int ligne = 4;
const int colonne = 5;
/**
 * @struct TableJeu
 * Structure représentant une table de jeu.
 * Elle contient un tableau de cartes de jeu de taille 4x5.
 */
struct TableJeu
{
    struct CarteJeu valeur[4][5];
};

struct TableJeu table; // table de jeu

bool finCorrecte = true; // fin du tour correcte
bool finjeu = false;     // fin du jeu

int deroulement = 0; // numéro du joueur qui joue
int nbcarteposé = 0; // nombre de carte posé
int manche = 1;      // numéro de la manche

int main(int argc, char *argv[])
{
    srand(time(NULL));
    message[TAILLEMAX] = malloc(sizeof(char) * TAILLEMAX);
    int sockfd, newsockfd, portno, clilen;
    char buffer[TAILLEMAX], action[12], fic[25], fic2[25], tt[25], proto[10];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, port NBJOUEUR NBROBOT\n");
        exit(1);
    }
    if (argc == 3)
    {
        NB_JOUEUR = atoi(argv[2]);
        NB_ROBOT = 0;
    }
    if (argc == 4)
    {
        NB_JOUEUR = atoi(argv[2]);
        NB_ROBOT = atoi(argv[3]);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // tant que la liste de joueur n'est pas remplie on accepte les connexions
    int cpt = 0;
    while (cpt < NB_JOUEUR)
    {
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");
        else
        {
            printf("client connecté\n");
            ListeJoueur[cpt] = newsockfd;
            cpt++;
        }
    }
    printf("début partie\n");
    generationCartedeJeu();
    distributionCarte();
    AffichageTableToutJoueur();
    // attendre le message d'un joueur

    bool memeTour = false;
    while (!finjeu)
    {
        finCorrecte = true;
        bzero(buffer, TAILLEMAX);
        bzero(message, TAILLEMAX);

        if (!memeTour)
        {
            message[0] = 'D';
            n = write(ListeJoueur[deroulement], message, sizeof(message));
        }

        n = read(ListeJoueur[deroulement], buffer, TAILLEMAX);
        if (n < 0)
            error("ERROR reading from socket");
        else
        {
            traitementMessage(buffer, deroulement);
            if (finCorrecte == true)
            {
                memeTour = false;
                if (buffer[0] != 'A')
                {
                    deroulement++;
                }
                if (deroulement == NB_JOUEUR)
                {
                    deroulement = 0;
                    // tour des robots

                    for (size_t i = 0; i < NB_ROBOT; i++)
                    {
                        printf("tour des robots\n");
                        int numrobot = NB_JOUEUR + i;
                        tourRobot(numrobot);
                    }
                }
            }
            else
            {
                memeTour = true;
            }
        }
    }

    close(sockfd);
    close(newsockfd);
    return 0;
}

void generationCartedeJeu()
{
    for (size_t i = 0; i < 104; i++)
    {
        int valeur = i + 1;
        ListeCarteJeu[i].valeur = i + 1;
        int chiffre = valeur % 10;
        int quotient = valeur / 10;
        if (chiffre == 5)
        {
            ListeCarteJeu[i].nbTaureau = 2;
        }
        else if (chiffre == 0)
        {
            ListeCarteJeu[i].nbTaureau = 3;
        }
        else if (i == 55)
        {
            ListeCarteJeu[i].nbTaureau = 7;
        }
        else if (chiffre == quotient)
        {
            ListeCarteJeu[i].nbTaureau = 5;
        }
        else
        {
            ListeCarteJeu[i].nbTaureau = 1;
        }
    }
}
void AffichageTableToutJoueur()
{
    bzero(message, TAILLEMAX);
    int n;
    strcpy(message, "T;");
    for (size_t i = 0; i < ligne; i++)
    {
        for (size_t j = 0; j < colonne; j++)
        {
            table.valeur[i][j].valeur = 0;
            table.valeur[i][j].nbTaureau = 0;
            char tempStr[10]; // Utilisez une taille appropriée pour stocker des entiers sous forme de chaînes
            sprintf(tempStr, "%dH%d;", table.valeur[i][j].valeur, table.valeur[i][j].nbTaureau);
            strcat(message, tempStr);
        }
    }
    for (size_t i = 0; i < NB_JOUEUR; i++)
    {
        n = write(ListeJoueur[i], message, sizeof(message));
    };
}

void traitementMessage(char *buffer, int deroulement)
{
    char choix = buffer[0];
    if (choix == 'A')
    {
        AffichageTable(deroulement);
        finCorrecte = true;
    }
    else if (choix == 'P')
    {
        choixCarte(buffer);
        finCorrecte = true;
        fintour();
    }
    else
    {
        printf("Erreur de message");
        finCorrecte = false;
    }
}

void AffichageTable(int numJoueur)
{
    printf("Affichage de la table \n");
    bzero(message, TAILLEMAX);
    int n;
    strcpy(message, "T;");
    for (size_t i = 0; i < ligne; i++)
    {
        for (size_t j = 0; j < colonne; j++)
        {
            char tempStr[10]; // Utilisez une taille appropriée pour stocker des entiers sous forme de chaînes
            sprintf(tempStr, "%dH%d;", table.valeur[i][j].valeur, table.valeur[i][j].nbTaureau);
            strcat(message, tempStr);
        }
    }
    n = write(ListeJoueur[numJoueur], message, sizeof(message));
}

void distributionCarte()
{
    printf("Distribution des cartes\n");
    bool dejaDist = false;
    struct CarteJeu ListeCarteJeuTemp[104];
    int indexTemp = 0;
    for (size_t i = 0; i < NB_JOUEUR + NB_ROBOT; i++)
    {
        bzero(message, TAILLEMAX);
        strcpy(message, "A;");
        for (size_t j = 0; j < 10; j++)
        {
            int random = 0;
            do
            {
                dejaDist = false;
                random = rand() % 104;
                for (size_t k = 0; k < indexTemp; k++)
                {
                    if (ListeCarteJeuTemp[k].valeur == ListeCarteJeu[random].valeur)
                    {
                        dejaDist = true;
                    }
                }
            } while (dejaDist == true);
            ListeCarteJeuTemp[indexTemp] = ListeCarteJeu[random];
            indexTemp++;
            char tempStr[10]; // Utilisez une taille appropriée pour stocker des entiers sous forme de chaînes
            sprintf(tempStr, "%d;", ListeCarteJeu[random].valeur);
            ListeJoueurStruct[i].ListeCarte[j] = ListeCarteJeu[random];
            strcat(message, tempStr);
        }

        int n = write(ListeJoueur[i], message, sizeof(message));
    }
}

void choixCarte(char *message)
{
    printf("Choix de la carte\n");
    char *token = strtok(message, ";");
    token = strtok(NULL, ";");
    int carte = atoi(token);
    token = strtok(NULL, ";");
    int position = atoi(token);
    printf("carte : %d, position : %d\n", carte, position - 1);
    bool cartePosée = false;
    for (int col = 0; col < colonne; col++)
    {
        if (table.valeur[position - 1][col].valeur == 0 && cartePosée == false)
        {
            table.valeur[position - 1][col].valeur = carte;
            table.valeur[position - 1][col].nbTaureau = ListeCarteJeu[carte - 1].nbTaureau;
            cartePosée = true;
        }
    }
    // carte non posable donc affichage point malus et mise a jour de la table
    if (!cartePosée)
    {
        int totaltete = 0;
        for (int col = 0; col < colonne; col++)
        {
            totaltete += table.valeur[position - 1][col].nbTaureau;
            table.valeur[position - 1][col].valeur = 0;
            table.valeur[position - 1][col].nbTaureau = 0;
        }
        table.valeur[position - 1][0].valeur = carte;
        table.valeur[position - 1][0].nbTaureau = ListeCarteJeu[carte - 1].nbTaureau;

        ListeJoueurStruct[deroulement].score += totaltete;
    }
    /*char messagePose [TAILLEMAX];
    bzero(messagePose, TAILLEMAX);
    strcpy(messagePose, "M;");
    strcpy(messagePose,"Joueur "); // Utilisez une taille appropriée pour stocker des entiers sous forme de chaînes
    sprintf(messagePose, "%d;", deroulement);
    strcpy(messagePose," a posé la carte "); // Utilisez une taille appropriée pour stocker des entiers sous forme de chaînes
    sprintf(messagePose, "%d;", carte);

    for(int i = 0; i < NB_JOUEUR;i++){
        int n = write(ListeJoueur[i], messagePose, sizeof(messagePose));
    }*/
}

void tourRobot(int numrobot)
{
    int nbcarterestant = 0;
    for (int i = 0; i < 10; i++)
    {
        if (ListeJoueurStruct[numrobot].ListeCarte[i].valeur != 0)
        {
            nbcarterestant++;
        }
    }
    int choixCarte = 0;
    do
    {
        choixCarte = rand() % nbcarterestant;
    } while (ListeJoueurStruct[numrobot].ListeCarte[choixCarte].valeur == 0);

    choixpositionRobot(choixCarte, ListeJoueurStruct[numrobot].ListeCarte[choixCarte].valeur, numrobot);
}

void choixpositionRobot(int choixcarte, int valeurcarte, int numrobot)
{
    printf("carte : %d\n", valeurcarte);

    int position = 0;
    bool trouve = false;
    for (int i = 0; i < ligne; i++)
    {
        if (table.valeur[i][0].valeur == 0 && trouve == false)
        {
            table.valeur[i][0].valeur = valeurcarte;
            table.valeur[i][0].nbTaureau = ListeCarteJeu[valeurcarte - 1].nbTaureau;
            trouve = true;
        }
        else
        {
            for (int j = 0; j < colonne; j++)
            {
                if (table.valeur[i][j].valeur == 0 && table.valeur[i][j - 1].valeur > valeurcarte && trouve == false)
                {
                    table.valeur[i][j].valeur = valeurcarte;
                    table.valeur[i][j].nbTaureau = ListeCarteJeu[valeurcarte - 1].nbTaureau;
                    ListeJoueurStruct[numrobot].ListeCarte[choixcarte].valeur = 0;
                    trouve = true;
                }
            }
        }
    }
    if (trouve == false)
    {
        int totaltete = 0;
        int position = 0;
        for (int col = 0; col < colonne; col++)
        {
            totaltete += table.valeur[position][col].nbTaureau;
            table.valeur[position][col].valeur = 0;
            table.valeur[position][col].nbTaureau = 0;
        }
        table.valeur[position][0].valeur = valeurcarte;
        table.valeur[position - 1][0].nbTaureau = ListeCarteJeu[valeurcarte - 1].nbTaureau;
        ListeJoueurStruct[numrobot].score += totaltete;
    }
    fintour();
}

void fintour()
{
    nbcarteposé++;
    // creation d'un string cmdsys pour lancer le script shell
    char cmdsys[4096];
    strcpy(cmdsys, "./GestionScore.sh ");

    sprintf(cmdsys + strlen(cmdsys), "%d", manche);

    strcat(cmdsys, " ");

    if (nbcarteposé == 10 * (NB_JOUEUR + NB_ROBOT))
    {
        nbcarteposé = 0;
        for (int i = 0; i < NB_JOUEUR + NB_ROBOT; i++)
        {
            sprintf(cmdsys + strlen(cmdsys), "%d", ListeJoueurStruct[i].score);
            strcat(cmdsys, " ");
            if (ListeJoueurStruct[i].score == 66)
            {
                finjeu = true;
            }
        };
        system(cmdsys);
        manche++;
        if (!finjeu)
        {
            distributionCarte();
            resetTable();
        }
    }
}

void resetTable()
{

    for (int i = 0; i < ligne; i++)
    {
        for (int j = 0; j < colonne; j++)
        {
            table.valeur[i][j].valeur = 0;
            table.valeur[i][j].nbTaureau = 0;
        }
    }
}
