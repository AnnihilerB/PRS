#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"

#define NB_CARACTERISTIQUES 6

#ifdef PADAWAN

int verif_tableau(int *tableau, int tailleTableau, int objet){
	for (int i =0; i < tailleTableau; i++){
		if ( objet == tableau[i]){
			return 1;
		}
	}
	return 0;
}

void map_new (unsigned width, unsigned height)
{
  map_allocate (16, 16);

  for (int x = 0; x < 16; x++)
    map_set (x, 16 - 1, 0); // Ground

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
	//int nb_objets = (int)map_objects();
	int nb_objets = 0;
	fprintf(stderr, "%d\n", nb_objets);

	write(map, &largeur, sizeof(int));
	write(map, &hauteur, sizeof(int));

	//Création d'un tableau d'objets de taille nb_objets pour stocker les objets différents rencontrés.
	int *tableau_objets = NULL;
	int index = 0;
	tableau_objets = malloc(nb_objets * sizeof(int));

	//Initialisation du tableau  à -2 
	for (int i = 0; i < nb_objets; i++){
		tableau_objets[i] = -2;
	}

	//Parcours de l'ensemble de la carte et récupération des objets aux coordonées

	int taille_carte = largeur * hauteur;
	int tab_carte[taille_carte];

        int i = 0;
	for (int y = 0; y < hauteur; y++){
		for (int x = 0; x < largeur; x++){
			tab_carte[i] = map_get (x,y);
			//Vérification que l'objet courant est nouveau
			if (tab_carte[i] != -1 && verif_tableau(tableau_objets, nb_objets, tab_carte[i]) == 0){
				tableau_objets[index] = tab_carte[i];
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

	for (int i = 0; i < nb_objets; i++){
		fprintf(stderr, "Debut objets\n");
		fprintf(stderr, "%d\n", tableau_objets[i]);
		char *nom_objet = map_get_name(tableau_objets[i]);
		
		tab_cara[0] = strlen(nom_objet);
		tab_cara[1] = map_get_frames(tableau_objets[i]);
		tab_cara[2] = map_get_solidity(tableau_objets[i]);
		tab_cara[3] = map_is_destructible(tableau_objets[i]);
		tab_cara[4] = map_is_collectible(tableau_objets[i]);
		tab_cara[5] = map_is_generator(tableau_objets[i]);

		write(map, &tab_cara, NB_CARACTERISTIQUES * sizeof(int));
	
		fprintf(stderr, "fin objets\n");

	}
	fprintf(stderr, "Done\n" );
	close(map);
}

void map_load (char *filename){

	int largeur;
	int hauteur;
	int nb_objets;

	int objet;

	int map = open(filename, O_RDONLY);

	//Lecture des informations de base : largeur, hauteur et nb_objets
	read(map, &largeur, sizeof(int));
	read(map, &hauteur, sizeof(int));
	read(map, &nb_objets, sizeof(int));

	fprintf(stderr, "largeur %d hauteur %d nb bjets %d\n", largeur, hauteur, nb_objets);

	map_allocate(largeur, hauteur);

	//Parcours de la matrice et placement des objets sur la carte
	for (int y = 0; y < hauteur; y++){
		for (int x = 0; x < largeur; x++){
			read(map, &objet, sizeof(int));
			map_set(x, y, objet);
		}
	}
	fprintf(stderr, "Fin du placement des objets\n");

	int taille_nom;
	int nb_sprites;
	int collectible;
	int destructible;
	int generator;
	int solid;

	fprintf(stderr, "Début begin\n");

	map_object_begin(nb_objets);

	for (int i = 0; i < nb_objets; i++){
		fprintf(stderr, "Debut for\n");
		read(map, &taille_nom, sizeof(int));
		fprintf(stderr, "taille nom %d\n", taille_nom);
		char nom_objet[taille_nom];
		
		read(map, &nom_objet, taille_nom *sizeof(char));
		read(map, &nb_sprites, sizeof(int));
		read(map, &solid, sizeof(int));
		read(map, &destructible, sizeof(int));
		read(map, &collectible, sizeof(int));
		read(map, &generator, sizeof(int));

		fprintf(stderr, "object add %d \n", i);
		fprintf(stderr, "%s\n", nom_objet);
		map_object_add(nom_objet, nb_sprites, MAP_OBJECT_SOLID);
		fprintf(stderr, "fin objet add\n");
	}
	map_object_end();
	fprintf(stderr, "Fin\n");
}

#endif
