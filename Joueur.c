#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#define TAILLEMAX 4096

char *message[TAILLEMAX];

/**
 * @struct CarteJeu
 * @brief Structure représentant une carte de jeu.
 *
 * Cette structure contient les informations relatives à une carte de jeu,
 * notamment sa valeur et le nombre de taureaux associés.
 */
struct CarteJeu
{
    int valeur;
    int nbTaureau;
};

/**
 * @struct Joueur
 * @brief Représente un joueur avec sa liste de cartes et son score.
 */
struct Joueur
{
    struct CarteJeu ListeCarte[10]; /**< La liste des cartes du joueur. */
    int score;                      /**< Le score du joueur. */
};

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

void error(char *msg)
{
    perror(msg);
    exit(0);
}

struct TableJeu *tableJeu; // table de jeu
struct Joueur *joueur;     // joueur
// fonction
/**
 * @fn void choixCarte()
 * @brief Fonction permettant de choisir une carte à poser.
 */
void choixCarte();
/**
 * @fn void AjoutMain(char* buffer);
 * @brief Fonction permettant d'ajouter les cartes à la main du joueur.
 * @param buffer Le message reçu du serveur.
 */
void AjoutMain(char *buffer);
/**
 * @fn void AffichageTable(char* buffer);
 * @brief Fonction permettant d'afficher la table de jeu.
 * @param buffer Le message reçu du serveur.
 */
void AffichageTable(char *buffer);
/**
 * @fn void traitementMessage(char* buffer);
 * @brief Fonction permettant de traiter les messages reçus du serveur.
 * @param buffer Le message reçu du serveur.
 */
void traitementMessage(char *buffer);
/**
 * @fn void AffichageMain();
 * @brief Fonction permettant d'afficher la main du joueur.
 */
void AffichageMain();
/**
 * @fn int generationTete(int valeur);
 * @brief Fonction permettant de générer la valeur de la tête d'une carte.
 * @param valeur La valeur de la carte.
 * @return La valeur de la tête de la carte.
 */
int generationTete(int valeur);
/**
 * @fn void PoserCarte(char* choixcarte,char *choixposition);
 * @brief Fonction permettant de poser une carte sur la table de jeu.
 * @param choixcarte La carte à poser.
 * @param choixposition La position où poser la carte.
 */
void PoserCarte(char *choixcarte, char *choixposition);
/**
 * @fn bool verifTable(char* choixcarte,char * choixposition);
 * @brief Fonction permettant de vérifier si la carte peut être posée sur la table de jeu.
 * @param choixcarte La carte à poser.
 * @param choixposition La position où poser la carte.
 * @return true si la carte peut être posée, false sinon.
 */
bool verifTable(char *choixcarte, char *choixposition);
/**
 * @fn void Messagesys(char* buffer);
 * @brief Fonction permettant d'afficher les messages du serveur.
 * @param buffer Le message reçu du serveur.
 */
void Messagesys(char *buffer);

int sockfd;                   // socket de connexion
char ancienbuffer[TAILLEMAX]; // ancien message reçu du serveur
int main(int argc, char *argv[])
{
    message[TAILLEMAX] = malloc(sizeof(char) * TAILLEMAX);
    tableJeu = malloc(sizeof(struct TableJeu));
    joueur = malloc(sizeof(struct Joueur));
    int portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[TAILLEMAX];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    printf("Connecté");
    while (1)
    {
        // recevoir un message du serveur de taille TAILLEMAX
        bzero(buffer, TAILLEMAX);
        n = read(sockfd, buffer, TAILLEMAX);
        if (n < 0)
            error("ERROR reading from socket");
        // Ajouter un caractère nul à la fin de la chaîne
        buffer[n] = '\0';
        strcpy(ancienbuffer, buffer);
        traitementMessage(buffer);
    }

    return 0;
}
void traitementMessage(char *buffer)
{
    char choix = buffer[0];
    if (choix == 'A')
    { // ajout de la main
        AjoutMain(buffer);
    }
    else if (choix == 'M')
    { // message du serveur
      // Messagesys(buffer);
    }
    else if (choix == 'T')
    { // affichage de la table
        AffichageTable(buffer);
    }
    else if (choix == 'D')
    { // gestion debut tour
        bool mauvaischoix = false;
        do
        {
            mauvaischoix = false;
            printf("C'est à vous de jouer\n");
            // recuperer le choix du joueur
            char choix[2];
            printf("Que veux-tu faire ? ( M = voir la main, A = pour actualiser et voir la table de jeu, P = Poser une carte )\n");
            scanf("%s", choix);
            // envoyer le choix au serveur
            if (choix[0] == 'M' || choix[0] == 'm')
            { // affichage de la main
                printf("Affichage de la main\n");
                AffichageMain();
                traitementMessage(ancienbuffer);
            }
            else if (choix[0] == 'A' || choix[0] == 'a')
            { // affichage de la table
                //system("clear");
                if(choix[0] == 'a'){
                    choix[0] = 'A';
                }
                int n = write(sockfd, choix, sizeof(choix));
                bzero(buffer, TAILLEMAX);
                n = read(sockfd, buffer, TAILLEMAX);
                if (n < 0)
                    error("ERROR reading from socket");
                AffichageTable(buffer);
            }
            else if (choix[0] == 'P' || choix[0] == 'p')
            { // poser une carte
                system("clear");
                bool retour = false;
                bool verif = false;
                char choixcarte[2];
                char choixposition[2];
                do
                {
                    retour = false;
                    AffichageMain();
                    printf("Choix de la carte\n");

                    bzero(choixcarte, 2);
                    scanf("%s", choixcarte);

                    printf("Choix de la position ou R pour revenir au choix de carte\n");

                    bzero(choixposition, 2);
                    scanf("%s", choixposition);
                    if (strcmp(choixposition, "R") == 0)
                    {
                        retour = true;
                    }
                    else
                    {
                        verif = verifTable(choixcarte, choixposition);
                        if (verif == false)
                        {
                            retour = true;
                        }
                    }
                } while (retour && !verif);
                PoserCarte(choixcarte, choixposition);
            }
            else
            {
                printf("Erreur de message\n");
                mauvaischoix = true;
            }
        } while (mauvaischoix);
    }
    else
    {
        printf("Erreur de message");
    }
}
void Messagesys(char *buffer)
{
    buffer[0] = ' '; // on enleve le M
    buffer[1] = ' '; // on enleve le ;
    printf("Message du serveur : %s\n", buffer);
}

bool verifTable(char *choixcarte, char *choixposition)
{
    bool ok = false;
    int position = atoi(choixposition);
    int valeurcarte = atoi(choixcarte);
    if (tableJeu->valeur[position - 1][colonne - 1].valeur != 0)
    {
        printf("fin de serie voulez vous prendre les points ? Y/N\n");
        char choix[2];
        scanf("%s", choix);
        if (choix[0] == 'Y')
        {
            ok = true;
        }
    }
    else
    {
        for (int col = 0; col < colonne; col++)
        {
            if (tableJeu->valeur[position - 1][col].valeur == 0)
            {
                if (col == 0 || (col != 0 && joueur->ListeCarte[valeurcarte].valeur < tableJeu->valeur[position - 1][col - 1].valeur))
                {
                    ok = true;
                }
            }
        }
    }

    if (ok == false)
    {
        printf("Position non valide\n");
    }
    return ok;
}

void AffichageTable(char *buffer)
{
    printf("Affichage de la table \n");
    int val = 0;
    bool tete = false;
    bool fincarte = false;
    int col = 0;
    int li = 0;
    printf("Carte :");
    for (int i = 2; buffer[i] != '\0'; i++)
    {
        if (buffer[i] == ';')
        {
            fincarte = true;

            col++;
            if (col == colonne)
            {
                if (li != ligne - 1)
                {
                    printf("\n Carte :");
                }
                else
                {
                    printf("\n");
                }
                col = 0;
                li++;
            }
            else
            {
                printf(" ; Carte :");
            }
        }
        else if (buffer[i] == 'H')
        {
            tete = true;
            printf(" tete : ");
        }
        else if (tete == false)
        {
            val += atoi(&buffer[i]);
            printf("%c", buffer[i]);
        }
        else if (tete == true)
        {
            tableJeu->valeur[li][col].valeur = val;
            tableJeu->valeur[li][col].nbTaureau = atoi(&buffer[i]);
            val = 0;
            printf("%c", buffer[i]);
            tete = false;
        }
    }
}

void AjoutMain(char *buffer)
{
    printf("Ajout de la main\n");
    int numcarte = 0;
    char *valeur = strtok(buffer, ";");
    while (valeur != NULL)
    {
        if (valeur != 'A')
        {
            joueur->ListeCarte[numcarte].valeur = atoi(valeur);
            numcarte++;
        }
        valeur = strtok(NULL, ";");
    }
    AffichageMain();
};

void AffichageMain()
{
    printf("Affichage de la main\n");
    for (int i = 1; i <= 10; i++)
    {
        if (joueur->ListeCarte[i].valeur != 0)
            printf("Carte %d : %d - Tete : %d\n", i, joueur->ListeCarte[i].valeur, generationTete(joueur->ListeCarte[i].valeur));
    }
}

int generationTete(int valeur)
{
    int tete = 0;
    int chiffre = valeur % 10;
    int quotient = valeur / 10;
    if (chiffre == 5)
    {
        tete = 2;
    }
    else if (chiffre == 0)
    {
        tete = 3;
    }
    else if (valeur == 55)
    {
        tete = 7;
    }
    else if (chiffre == quotient)
    {
        tete = 5;
    }
    else
    {
        tete = 1;
    }
    return tete;
}

void PoserCarte(char *choixcarte, char *choixposition)
{
    int carte = atoi(choixcarte);
    if (carte > 9)
    {
        carte = 10;
    }
    char message[TAILLEMAX];
    bzero(message, TAILLEMAX);
    sprintf(message, "P;%d;%s", joueur->ListeCarte[carte].valeur, choixposition);
    int n = write(sockfd, message, sizeof(message));
    joueur->ListeCarte[carte].valeur = 0;
}