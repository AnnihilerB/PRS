#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#define MAP_OBJECT_AIR           0
#define MAP_OBJECT_SEMI_SOLID    1
#define MAP_OBJECT_SOLID         2

#define MAP_OBJECT_DESTRUCTIBLE  4
#define MAP_OBJECT_COLLECTIBLE   8
#define MAP_OBJECT_GENERATOR     16
#define NB_CARACTERISTIQUES 9

int lireEntierPositif (int fichierMap);
int getWidth(int fichierMap);
int getHeight(int fichierMap);
int getObjects(int fichierMap);
void printWidth(int fichierMap);
void printHeight(int fichierMap);
void printObjects(int fichierMap);
int supprimerListeObjets (int fichierMap);
int verificationArgumentsSetObjects (char *argv[], int n, int fichierMap);
int setObjects(char **argv, int n, int fichierMap);

enum ErrArgsSetObjects {ERRNOM, ERRTAILLE};
char* caracteristiques[] = {"solid", "semi-solid", "air", "collectible", "destructible", "generator",  "not-destructible", "not-collectible", "not-generator"};


int main (int argc, char** argv)
{
	if (argc > 2)	//doit avoir pour argument au moins le nom du fichier et un argument
	{
		int fichierMap = open(argv[1],O_RDWR, 0666);
		if (fichierMap != -1)
		{
			if (strcmp(argv[2], "--getinfos") == 0)
			{
				char* e = "largeur: ";
				write(1, e, strlen(e));
				printWidth(fichierMap);
				e = "hauteur: ";
				write(1, e, strlen(e));
				printHeight(fichierMap);
				e = "nombre d'objets: ";
				write(1, e, strlen(e));
				printObjects(fichierMap);
			}
			else if (strcmp(argv[2], "--getwidth") == 0)
				printWidth(fichierMap);
			else if (strcmp(argv[2], "--getheight") == 0)
				printHeight(fichierMap);
			else if (strcmp(argv[2], "--getobjects") == 0)
				printObjects(fichierMap);
			else if (strcmp(argv[2], "--setobjects") == 0)
			{
				int repVerificationArguments = verificationArgumentsSetObjects(&argv[3], argc-3, fichierMap);
				if (repVerificationArguments == ERRNOM)
				{
					char *e = "erreur noms arguments\n";
					write(2, e, strlen(e));
				}
				else if (repVerificationArguments == ERRTAILLE)
				{	
					char *e = "erreur taille des arguments\n";
					write(2, e, strlen(e));
				}
				else
				{	
					printf("c'est bon \n");
					setObjects(&argv[3], argc-3, fichierMap);
				}
			}
			else
			{
				char *e = "erreur arguments\n";
				write(2, e, strlen(e));
			}

			close(fichierMap);	 	
		}
		else
		{
			char *e = "erreur chargement fichier\n";
			write(2, e, strlen(e));
		}
	}
	return 0;
}
int verificationArgumentsSetObjects (char *argv[], int n, int fichierMap) //verifie
{
	if (n % 6 != 0 || n / 6 < getObjects(fichierMap))
		return ERRTAILLE;
	
	if (atoi(argv[1]) == 0)
		return ERRNOM;
	for (int i = 2; i < n; i++)	//on commence à 2 car le premier est le chemin de l'image et un chiffre
	{
		int p = 0;
		if (i % 6 == 0)		//si c'est un chemin d'image
		{
			i++;
			if (atoi(argv[i]) == 0)
				return ERRNOM;
			i ++; 
		}
			
		while (p < NB_CARACTERISTIQUES)
		{
			if (strcmp(argv[i], caracteristiques[p]) == 0)
				break;
			p ++;
		}
		if (p == NB_CARACTERISTIQUES)		//donc si l'argument n'est pas reconnu
			return ERRNOM;
	}
}
int setObjects(char **argv, int n, int fichierMap)
{
	int hauteurMap = getHeight(fichierMap);
	int largeurMap = getWidth(fichierMap);
	int saveMap[hauteurMap][largeurMap];
	lseek(fichierMap, 3*sizeof(int), SEEK_SET);
	for (int i = 0; i < hauteurMap; i++)     //on sauvegarde la matrice
	      read(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	
	ftruncate(fichierMap, 2*sizeof(int));		//on supprime tout sauf la largeur et la hauteur
	printf("%s\n", strerror(errno));
	lseek(fichierMap, 0, SEEK_END);
	
	int tmp = n/6;
	write(fichierMap, &tmp, sizeof(int));//on écrit la nouvelle valeur du nombre d'objet
	for (int i = 0; i < hauteurMap; i++)		//on récrit la matrice
	      write(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	
	//puis on écrit la nouvelle liste
	for (int i = 0; i < n; i++)
	{
	      if (i % 6 == 0)
		      write(fichierMap, argv[i], strlen(argv[i]));
	      else
	      {
		      int p = 0;
		      while (p < NB_CARACTERISTIQUES)
		      {
			      if (strcmp(argv[i], caracteristiques[p]) == 0)
			      {
				      if (p >= 0 && p < 3)	//solid, air, semi-solid
					      write(fichierMap, &p, sizeof(int));
				      else
				      {
					      int r;
					      if (p == 3)
						r = MAP_OBJECT_COLLECTIBLE;
					      else if (p == 4)
						r = MAP_OBJECT_DESTRUCTIBLE;
					      else
						r = MAP_OBJECT_GENERATOR;
					      write(fichierMap, &r, sizeof(int));
				      }
				      break;
			      }
			      p ++;
		      }
	      }
	}
	
	
}
int lireEntierPositif (int fichierMap)   //positif pour avoir un message d'erreur qui est -1
{
	int nb;
	int rep = read(fichierMap, &nb, sizeof(int));
	if (rep == - 1 || rep == 0)
		return -1;
	return nb;
}
int getWidth(int fichierMap)
{
	lseek(fichierMap, 0, SEEK_SET);
	return lireEntierPositif(fichierMap);
}
void printWidth(int fichierMap)
{
	int nb = getWidth(fichierMap);
	if (nb == -1)
	{
		char* e = "erreur chargement largeur\n";
		write(2, e, strlen(e));
	}
	else
		printf("%d\n", nb);
}
int getHeight(int fichierMap)
{
	lseek(fichierMap, sizeof(int), SEEK_SET);
	return lireEntierPositif(fichierMap);
}
void printHeight(int fichierMap)
{
	int nb = getHeight(fichierMap);
	if (nb == -1)
	{
		char* e = "erreur chargement hauteur\n";
		write(2, e, strlen(e));
	}
	else
		printf("%d\n", nb);
}
int getObjects(int fichierMap)
{
	lseek(fichierMap, 2*sizeof(int), SEEK_SET);
	return lireEntierPositif(fichierMap);
}
void printObjects(int fichierMap)
{
	
	int nb = getObjects(fichierMap);
	if (nb == -1)
	{
		char* e = "erreur chargement nombre d\'objet\n";
		write(2, e, strlen(e));
	}
	else
		printf("%d\n", nb);
}




