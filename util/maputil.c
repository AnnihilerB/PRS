#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>

#define MAP_OBJECT_AIR           0
#define MAP_OBJECT_SEMI_SOLID    1
#define MAP_OBJECT_SOLID         2

#define MAP_OBJECT_DESTRUCTIBLE  4
#define MAP_OBJECT_COLLECTIBLE   8
#define MAP_OBJECT_GENERATOR     16
#define NB_CARACTERISTIQUES 9
#define MAP_OBJECT_NONE -1
#define NB_OBJECT_MAX 10

int lireEntierPositif (int fichierMap);
int getWidth(int fichierMap);
int getHeight(int fichierMap);
int getObjects(int fichierMap);
void printWidth(int fichierMap);
void printHeight(int fichierMap);
void printObjects(int fichierMap);
int supprimerListeObjets (int fichierMap);
int verificationArgumentsSetObjects (char *argv[], int n, int fichierMap);
void setWidthHeight(int longueur, char type, int fichierMap);
int setObjects(char **argv, int nombre_args, int fichierMap);
void pruneObjects(int fichierMap);


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
			
			else if (strcmp(argv[2], "--setwidth") ==  0) 
			  setWidthHeight(atoi(argv[3]), 'w', fichierMap);			
			else if (strcmp(argv[2], "--setheight") ==  0)
			  setWidthHeight(atoi(argv[3]), 'h', fichierMap);	       					
			else if (strcmp(argv[2], "--pruneobjects") ==  0)
			  pruneObjects(fichierMap);
		
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
int verificationArgumentsSetObjects (char *argv[], int n, int fichierMap)
{
	if (n % 6 != 0 || n / 6 < getObjects(fichierMap))
		return ERRTAILLE;
	
	if (atoi(argv[1]) == 0)
		return ERRNOM;
	for (int i = 2; i < n; i++)	//on commence à 2 car le premier est le chemin de l'image et un chiffre
	{
		//Si c'est une image
		if (i % 6 == 0)
		{
			i++;
			if (atoi(argv[i]) == 0)
				return ERRNOM;
			i++;
		}
		//Vérification que les caractéristiques sont correctes.
		int index_cara = 0;
		while (index_cara < NB_CARACTERISTIQUES)
		{
			if (strcmp(argv[i], caracteristiques[index_cara]) == 0)
				break;
			index_cara++;
		}
		//Si argument non reconnu
		if (index_cara == NB_CARACTERISTIQUES)
			return ERRNOM;
	}
}
int setObjects(char **argv, int nombre_args, int fichierMap)
{
	int hauteurMap = getHeight(fichierMap);
	int largeurMap = getWidth(fichierMap);
	int saveMap[hauteurMap][largeurMap];
	lseek(fichierMap, 3*sizeof(int), SEEK_SET);
	//Sauvegarde de la matrice
	for (int i = 0; i < hauteurMap; i++)     
	      read(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	
	//Suppression de tout sauf largeur et hauteur
	ftruncate(fichierMap, 2*sizeof(int));		
	printf("%s\n", strerror(errno));
	lseek(fichierMap, 0, SEEK_END);
	
	int tmp = nombre_args/6;
	write(fichierMap, &tmp, sizeof(int));//on écrit la nouvelle valeur du nombre d'objet
	for (int i = 0; i < hauteurMap; i++)		//on récrit la matrice
	      write(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	
	//Ecriture de la nouvelle liste
	for (int i = 0; i < nombre_args; i++)
	{
		//Ecriture du nom des fichiers d'images
	      if (i % 6 == 0)
		      write(fichierMap, argv[i], strlen(argv[i]));
	      //Ecriture des caractéristiques.
	      else
	      {
		      int index_cara = 0;
		      while (index_cara < NB_CARACTERISTIQUES)
		      {
			      if (strcmp(argv[i], caracteristiques[index_cara]) == 0)
			      {
			    	  //Solidité de l'objet
				      if (index_cara >= 0 && index_cara < 3)
					      write(fichierMap, &index_cara, sizeof(int));
				      else
				      {
					      int flags;
					      if (index_cara == 3)
						flags = MAP_OBJECT_COLLECTIBLE;
					      else if (index_cara == 4)
						flags = MAP_OBJECT_DESTRUCTIBLE;
					      else
						flags = MAP_OBJECT_GENERATOR;
					      write(fichierMap, &flags, sizeof(int));
				      }
				      break;
			      }
			      index_cara ++;
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

void setWidthHeight(int longueur, char type, int fichierMap){
  
        int hauteurMap = getHeight(fichierMap); //on sauvegarde tous les paramètres du fichier map dans des variables.
	int largeurMap = getWidth(fichierMap);
	int nb_objects = getObjects(fichierMap);
	
	int saveMap[hauteurMap][largeurMap];
	lseek(fichierMap, 3*sizeof(int),SEEK_SET);
	for (int i = 0; i < hauteurMap; i++){   //sauvegarde de la matrice.
	  read(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	}
	
	int taillechaine[nb_objects];
	char *chaine[nb_objects];
	int nb_caract=5;
	int caract[nb_objects][nb_caract];
	
	for(int i = 0; i<nb_objects; i++){ //sauvegarde de la taille de la chaine de caractère, du nom de l'objet et de ses caractéristiques.
	  read(fichierMap,&taillechaine[i],sizeof(int));
	  chaine[i]=malloc(taillechaine[i]*sizeof(char));
	  for(int j=0; j<taillechaine[i]; j++){ 
	    read(fichierMap, &chaine[i][j], sizeof(char));
	  }	  
	  for(int k=0; k<nb_caract;k++){
	    read(fichierMap, &caract[i][k], sizeof(int));
	  }
	}

	int old_largeur=largeurMap;
	int old_hauteur=hauteurMap;
	
	if(type=='w')
	  largeurMap = longueur;
	else
	  hauteurMap = longueur;	
	
	int newMap[hauteurMap][largeurMap]; //création nouvelle matrice avec la nouvelle valeur de largeur ou de hauteur.
	memset(newMap, MAP_OBJECT_NONE, sizeof(newMap[hauteurMap][largeurMap])*hauteurMap*largeurMap);

	if(largeurMap>old_largeur)
	  largeurMap=old_largeur;
	if(hauteurMap>old_hauteur)
	  hauteurMap=old_hauteur;
	
	for(int i=0; i<hauteurMap; i++){
	  for(int j=0; j<largeurMap; j++){
	    newMap[i][j]=saveMap[i][j]; //remplit de la matrice avec les éléments de l'ancienne.
	  }
	}

	if(type=='w')
	  largeurMap = longueur;
	else
	  hauteurMap = longueur;
	
	ftruncate(fichierMap, 0); //on efface le fichier entier.
        lseek(fichierMap, 0, SEEK_SET); //on se positionne au début du fichier.
        write(fichierMap, &largeurMap, sizeof(int)); //on réécrit tout.
        write(fichierMap, &hauteurMap, sizeof(int));
        write(fichierMap, &nb_objects, sizeof(int));
	
	for(int i=0; i<hauteurMap; i++){ //réécriture de la nouvelle matrice.
	  for(int j=0; j<largeurMap; j++){
	    write(fichierMap, &newMap[i][j], sizeof(int));
	  }
	}
	
	for(int i=0; i<nb_objects; i++){ 
	  write(fichierMap, &taillechaine[i], sizeof(int));
	  for(int j=0; j<taillechaine[i]; j++){
	    write(fichierMap, &chaine[i][j], sizeof(char));
	  }
	  for(int k=0; k<nb_caract; k++){
	    write(fichierMap, &caract[i][k], sizeof(int));
	  }
        }
	
	for(int i=0; i<nb_objects; i++){ //on free les mallocs du tableau contenant les noms des éléments.
	  free(chaine[i]);
	}
}

void pruneObjects(int fichierMap){
  
        int hauteurMap = getHeight(fichierMap);
	int largeurMap = getWidth(fichierMap);
	int nb_objects = getObjects(fichierMap);
	
	int saveMap[hauteurMap][largeurMap];
	lseek(fichierMap, 3*sizeof(int),SEEK_SET);
	for (int i = 0; i < hauteurMap; i++){   
	  read(fichierMap, &saveMap[i], largeurMap*sizeof(int));
	}
	
	int taillechaine[nb_objects];
	char *chaine[nb_objects];
	int nb_caract=5;
	int caract[nb_objects][nb_caract];
	
	for(int i = 0; i<nb_objects; i++){
	  read(fichierMap,&taillechaine[i],sizeof(int));
	  chaine[i]=malloc(taillechaine[i]*sizeof(char));
	  for(int j=0; j<taillechaine[i]; j++){ 
	    read(fichierMap, &chaine[i][j], sizeof(char));
	  }	  
	  for(int k=0; k<nb_caract;k++){
	    read(fichierMap, &caract[i][k], sizeof(int));
	  }
	}
	
	int compteur[NB_OBJECT_MAX];
	memset(compteur, 0, sizeof(int)*NB_OBJECT_MAX);
	int id[10]={0,1,2,3,4,5,6,7,8,9};	
	for(int i=0; i<hauteurMap; i++){ //pour chaque type d'éléments on compte son nombre d'occurences dans le fichier de la carte.
	  for(int j=0; j<largeurMap; j++){
	    for(int k=0; k<9; k++){
	      if(saveMap[i][j]==id[k]){
		compteur[k]++;
	      }
	    }
	  }
	}

	ftruncate(fichierMap, 0);
	lseek(fichierMap, 0, SEEK_SET);
	write(fichierMap, &largeurMap, sizeof(int));
	write(fichierMap, &hauteurMap, sizeof(int));

	int true_nb_objects=0;
	for(int i=0; i<9; i++){ //on détermine le nombre réel d'élements présent sur la carte. 
	  if(compteur[i]!=0){
	    true_nb_objects++;
	  }
	}
    
	write(fichierMap, &true_nb_objects, sizeof(int)); 
	
	int curseur=0;
	int object_not_erase[true_nb_objects];
	for(int i=0; i<9; i++){ //on place dans un tableau, les id des élements présent sur la carte.
	  if(compteur[i]!=0){
	    object_not_erase[curseur]=i;
	    curseur++;
	  }
	}
	
	for(int i=0; i<hauteurMap; i++){
	  write(fichierMap, &saveMap[i], largeurMap*sizeof(int)); 
	}

        for(int i=0; i<nb_objects; i++){
	  if(i==object_not_erase[i]){ //si l'id de l'élément est bien présent alors on l'écrit dans le fichier. 
	     write(fichierMap, &taillechaine[i], sizeof(int));
	     for(int j=0; j<taillechaine[i]; j++){
	       write(fichierMap, &chaine[i][j], sizeof(char));
	     }
	     for(int k=0; k<nb_caract; k++){
	       write(fichierMap, &caract[i][k], sizeof(int));
	     }
	    }
	}

	for(int i=0; i<nb_objects; i++){ 
	  free(chaine[i]);
	}
}


