#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#define NB_CARACTERISTIQUES 5
#define MAX_OBJETS 10

#ifdef PADAWAN

int verif_tableau(int tableau[], int objet){
	for (int i =0; i < MAX_OBJETS; i++){
		if ( objet == tableau[i]){
			return 1;
		}
	}
	return 0;
}

void map_new (unsigned width, unsigned height)
{
  map_allocate (16, 16);

  for (int x = 0; x < 16; x++){
    map_set (x, 16 - 1, 0); // Ground
  }
    
    map_set (3, 5, 3);
    map_set (4, 5, 3);
  
  for (int y = 0; y < 16 - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (16 -1, y, 1); // Wall
  }

  map_object_begin (4);

  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
  // Mur
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
  // Gazon
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
  // Marbre
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);

  map_object_end ();

}

void map_save (char *filename)
{
    
        filename = "saved.map";

	int map = open(filename, O_WRONLY | O_CREAT, 0666);

	//Récupération de largeur, hauteur et nb objets suivi de leur écriture dans le fichier.
	int hauteur = (int)map_height();
	int largeur = (int)map_width();
	int nb_objets = 0;

	write(map, &largeur, sizeof(int));
	write(map, &hauteur, sizeof(int));

	//Création d'un tableau d'objets de taille MAX_OBJETS pour stocker les objets différents rencontrés.
	int tableau_objets[MAX_OBJETS];

	//Initialisation du tableau  à -2 
	for (int i = 0; i < MAX_OBJETS; i++){
		tableau_objets[i] = -2;
	}

	//Parcours de l'ensemble de la carte et récupération des objets aux coordonées

	int taille_carte = largeur * hauteur;
	int tab_carte[taille_carte];

        int i = 0;
        int index = 0;
	for (int y = 0; y < hauteur; y++){
            for (int x = 0; x < largeur; x++){
                tab_carte[i] = map_get (x,y);
                //Vérification que l'objet courant est nouveau
                if (tab_carte[i] != -1 && verif_tableau(tableau_objets,tab_carte[i]) == 0){
                    fprintf(stderr, "Id de %d\n", tab_carte[i]);
                    tableau_objets[tab_carte[i]] = tab_carte[i];
                    index ++;
                    nb_objets++;
                }
                i++;
            }
	}
	
	write(map, &nb_objets, sizeof(int));
        printf("Objets : %d\n", nb_objets);
	write(map, &tab_carte, taille_carte * sizeof(int));
        

	//Ecriture pour chaque objet des informations le concernant

	int tab_cara[NB_CARACTERISTIQUES];

	for (int i = 0; i < MAX_OBJETS; i++){
            if (tableau_objets[i] != -2){
		fprintf(stderr, "Debut objets\n");
		fprintf(stderr, "%d\n", tableau_objets[i]);
                
                char *nom_objet = map_get_name(tableau_objets[i]);
                int taille_nom = strlen(nom_objet);
                
                fprintf(stderr, "Nom : %s taille : %d", nom_objet, taille_nom); 
                
                write(map, &taille_nom, sizeof(int));
                write(map, nom_objet, taille_nom * sizeof(char));
                
		tab_cara[0] = map_get_frames(tableau_objets[i]);
		tab_cara[1] = map_get_solidity(tableau_objets[i]);
		tab_cara[2] = map_is_destructible(tableau_objets[i]);
		tab_cara[3] = map_is_collectible(tableau_objets[i]);
		tab_cara[4] = map_is_generator(tableau_objets[i]);

		write(map, &tab_cara, NB_CARACTERISTIQUES * sizeof(int));
	
		fprintf(stderr, "fin objets\n");
            }
	}
	fprintf(stderr, "Done\n" );
	close(map);
}

void map_load (char *filename){

	int largeur = 0;
	int hauteur = 0;
	int nb_objets = 0;

	int objet = 0;

	int map = open(filename, O_RDONLY);

	//Lecture des informations de base : largeur, hauteur et nb_objets
	read(map, &largeur, sizeof(int));
	read(map, &hauteur, sizeof(int));
	read(map, &nb_objets, sizeof(int));
        

	map_allocate(largeur, hauteur);

	//Parcours de la matrice et placement des objets sur la carte
	for (int y = 0; y < hauteur; y++){
		for (int x = 0; x < largeur; x++){
			read(map, &objet, sizeof(int));
                        fprintf(stderr, "%d ", objet);
                        if (objet != -1)
                            map_set(x, y, objet);
                        
                }
                fprintf(stderr, "\n", objet);
	}

	int taille_nom = 0;
	int nb_sprites = 0;
	int collectible = 0;
	int destructible = 0;
	int generator = 0;
	int solid = 0;


	map_object_begin(nb_objets);

	for (int i = 0; i < nb_objets; i++){
		read(map, &taille_nom, sizeof(int));
		char nom_objet[taille_nom];
		
		read(map, &nom_objet, taille_nom *sizeof(char));
		read(map, &nb_sprites, sizeof(int));
		read(map, &solid, sizeof(int));
		read(map, &destructible, sizeof(int));
		read(map, &collectible, sizeof(int));
		read(map, &generator, sizeof(int));

                fprintf(stderr,"Nom : %s\n", nom_objet);
                //map_object_add (nom_objet, 1, MAP_OBJECT_SOLID);
                
		//fprintf(stderr, "fin objet add\n");
	}
	fprintf(stderr, "hors for\n");
        map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID);
        map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID);
        map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
	
        map_object_end();
}
#endif
