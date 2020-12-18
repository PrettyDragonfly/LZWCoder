#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define EMPTY_COMMAND 256
#define END_OF_STREAM 257
#define MAX_SIZE_DICO 4096

typedef struct {
    char *mots[MAX_SIZE_DICO];
    int nb_mots;
} dico_t;

typedef unsigned char octet_t;

static dico_t DICO;

int pfd[2];

void init_dictionnary() {
    for (int i = 0; i < 256; i++) {
        DICO.mots[i] = (char *)malloc(1);
        *DICO.mots[i] = i;
    }
    DICO.mots[EMPTY_COMMAND] = (char *)malloc(1);
    *DICO.mots[EMPTY_COMMAND] = ' ';
    DICO.mots[END_OF_STREAM] = (char *)malloc(1);
    *DICO.mots[END_OF_STREAM] = ' ';
    DICO.nb_mots = END_OF_STREAM + 1;
}

void empty_dictionnary(){
    printf("---- EMPTY DICTIONNARY ----\n");
    DICO.nb_mots = END_OF_STREAM + 1;
}

int EstPresent(char *mot) {
    for (int i = 0; i < DICO.nb_mots; i++)
    {
        if (strcmp(mot, DICO.mots[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

void ajouter(char *word, char *who) {
    printf("[%s] ajoute mot '%s' à la position %d\n", who, word,DICO.nb_mots);
    DICO.mots[DICO.nb_mots] = (char *)malloc(((strlen(word) + 1) * sizeof(char)));
    printf("[%s] Reserved %d bytes\n", who,(strlen(word) + 1) * sizeof(char));
    printf("[%s] Prepare to copy %d bytes from word to dico.\n", who,  strlen(word) + 1);

    strncpy(DICO.mots[DICO.nb_mots], word, strlen(word) + 1);

    printf("Copied\n");
    DICO.nb_mots++;
    printf("Nb mots dico %d dans le %s\n",DICO.nb_mots,who);
    printf("Ajout successful\n");
}

octet_t *prepare_triplet(int first, int second) {
    octet_t oc1 = first >> 4;

    octet_t oc2 = (first & 15) << 4;
    octet_t aux = second >> 8;
    oc2 = oc2 | aux;

    octet_t oc3 = second & 255;

    octet_t *triplet = (octet_t *)calloc(3, sizeof(octet_t));
    triplet[0] = oc1;
    triplet[1] = oc2;
    triplet[2] = oc3;

    return triplet;
}

int *unpack_triplet(octet_t *triplet) {
    int *couple = (int *)calloc(2, sizeof(int));

    couple[0] = (triplet[0] << 4) | (triplet[1] >> 4); // first 8 bits
    couple[1] = (triplet[1] & 0x0f) << 8 | triplet[2];

    return couple;
}

void emettre(const int code) {
    static int cpt = 0;
    static int couple[2];
    cpt += 1;
    if (cpt == 2)
    {
        cpt = 0;
        couple[1] = (code == -1) ? 32 : code;
        octet_t *triplet = prepare_triplet(couple[0], couple[1]);

        printf("Emission de (%d, %d)\n", couple[0], couple[1]);
        if (write(pfd[1], triplet, 3 * sizeof(octet_t)) == -1)
            erreur("write");
    }
    else
    {
        couple[0] = (code == -1) ? 32 : code;
    }
}

int *recevoir() {
    octet_t *triplet = (octet_t *)calloc(3, sizeof(octet_t));

    if (read(pfd[0], triplet, 3 * sizeof(octet_t)) == -1)
        erreur("read");

    int *couple = unpack_triplet(triplet);
    printf("Reception de (%d, %d)\n", couple[0], couple[1]);

    return couple;
}

void codeur(char *filename) {
    printf("Address DICO dans codeur: %p\n", &DICO);
    FILE *fd = fopen(filename, "r");

    if (fd == NULL) erreur("fopen");

    int compteur = 0;
    char caractere_suivant = fgetc(fd);
    char *string_caractere_suivant = (char *)malloc(2);
    string_caractere_suivant[0] = caractere_suivant;
    string_caractere_suivant[1] = '\0';

    char *string_symbole_courant = (char *)malloc(1);
    *string_symbole_courant = '\0';

    while (caractere_suivant != EOF) {
        char *mot_evalue = (char *)malloc(strlen(string_symbole_courant) + 1);
        strcpy(mot_evalue, string_symbole_courant);
        strcat(mot_evalue, string_caractere_suivant);

        int index = EstPresent(mot_evalue);

        if (index != -1) {
            string_symbole_courant = mot_evalue;
        }
        else {
            emettre(EstPresent(string_symbole_courant));
            compteur++;

            if (DICO.nb_mots >= MAX_SIZE_DICO) {
                printf("---- Empty dictionnary ----\n");
                empty_dictionnary();
                emettre(EMPTY_COMMAND);
                compteur++;
            }
            ajouter(mot_evalue, "codeur");
            strncpy(string_symbole_courant, string_caractere_suivant, 2);
        }

        caractere_suivant = fgetc(fd);
        string_caractere_suivant[0] = caractere_suivant;
    }

    if (compteur % 2 != 0) {
        printf("Je dois emettre un EOS\n");
        emettre(END_OF_STREAM);
    } else {
        printf("Je dois emettre deux EOS\n");
        emettre(END_OF_STREAM);
        emettre(END_OF_STREAM);
    }

    printf("Fin codeur\n");

    if (fclose(fd) == -1) erreur("close");

    exit(0);
}

void decodeur() {
    printf("Address DICO dans decodeur: %p\n", &DICO);

    if (close(pfd[1]) == -1) erreur("closepfd");

    FILE *res = fopen("Resultat.txt", "w+");
    if (res == NULL) erreur("fopen");

    int *couple = recevoir();
    int k = couple[0];										//On récupère le code du premier caractère du texte
    fprintf(res,"%c", (char)k);
    printf("Afficher %c\n", (char)k);

    char *w = (char *)malloc(2);
    w[0] = k;															//On place ce caractère dans une chaine
    w[1] = '\0';

    int sel = 1;

    while (couple[0] != 0 || couple[1] != 0) {
        k = couple[sel];

        if(k == END_OF_STREAM){
            break;
        }

        if (k == EMPTY_COMMAND) {
            empty_dictionnary();
        }
        else {
            if (k < DICO.nb_mots) {
                char *entry = DICO.mots[k];
                fprintf(res,"%s", entry);
                printf("Afficher %s\n", entry);

                char *new_entry = malloc(strlen(w) + 2);
                strncpy(new_entry, w, 2);
                new_entry[strlen(w)] = entry[0];

                ajouter(new_entry, "decodeur");
                w = entry;
            }
            else {
                char *new_entry = malloc(strlen(w) + 2);
                strncpy(new_entry, w, strlen(w));
                new_entry[strlen(w)] = w[0];

                fprintf(res,"%s", new_entry);
                printf("Afficher %s\n", new_entry);

                ajouter(new_entry, "decodeur");
                w = new_entry;
            }
        }

        if (sel == 1)	{										//le couple est entièrement traité, on peut en récupérer un nouveau
            couple = recevoir();
            sel = 0;
        }
        else {
            sel = 1;
        }


    }

    fprintf(res,"\n"); // Flush buffer

    if (fclose(res) == -1) erreur("close");

    if (close(pfd[0]) == -1) erreur("close");

    if (wait(NULL) == -1) erreur("wait"); //attente du codeur
}

int main(int argc, char *argv[]) {
    if (argc != 2) erreur("argc");

    init_dictionnary();

    if (pipe(pfd) == -1) erreur("pipe");

    codeur(argv[1]);
    /*switch (fork()) {
        case -1:
            erreur("fork");
        case 0:
            codeur(argv[1]);
    }

    decodeur();*/

    return 0;
}
