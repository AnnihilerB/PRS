#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "map.h"
#include "error.h"

#define NB_CARACTERISTIQUES 5
#define MAX_OBJETS 10

#ifdef PADAWAN



int generer_flags(int solid, int collectible, int destructible, int generator){
    int flags = 0;
    //Ajout dans tous les case de la solidité
    flags |= solid;
    
    //Ajout des flags à 1 uniquement.
    if (collectible)
        flags |= MAP_OBJECT_COLLECTIBLE;
    if (destructible)
        flags |= MAP_OBJECT_DESTRUCTIBLE;
    if (generator)
        flags |= MAP_OBJECT_GENERATOR;
    
    return flags;
}
    

void map_new (unsigned width, unsigned height)
{
  map_allocate (16, 16);

  for (int x = 0; x < 16; x++){
    map_set (x, 16 - 1, 0); // Sol
  }
    map_set (5,5, 2);//un carré de marble
    map_set(6,5,3); //Carré de grass
  
  
  for (int y = 0; y < 16 - 1; y++) {
    map_set (0, y, 1); // Wall
    map_set (16 -1, y, 1); // Wall
  }

  map_object_begin (4);

  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID); // 0

  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID); // 1

  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE); // 2
  
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID); // 3

  map_object_end ();

}

void map_save (char *filename)
{
        //Numéro d'erreur.
        errno = 0;
    
        //Ouverture du fichier.
        filename = "saved.map";
	int map = open(filename, O_WRONLY | O_CREAT, 0666);
        if (map == -1){
            fprintf(stderr, "Error creating the save file. %s", strerror(errno));
            exit(errno);
        }
            
	//Récupération de largeur, hauteur et nb objets suivi de leur écriture dans le fichier.
	int hauteur = (int)map_height();
	int largeur = (int)map_width();
	int nb_objets = (int)map_objects();

	write(map, &largeur, sizeof(int));
	write(map, &hauteur, sizeof(int));
        write(map, &nb_objets, sizeof(int));

	//Parcours de l'ensemble de la carte et récupération des objets aux coordonées
	int taille_carte = largeur * hauteur;
	int tab_carte[taille_carte];

        int index = 0;
	for (int y = 0; y < hauteur; y++){
            for (int x = 0; x < largeur; x++){
                tab_carte[index] = map_get (x,y);
                index++;
            }
        }
	write(map, &tab_carte, taille_carte * sizeof(int));
        

	//Ecriture pour chaque objet des informations le concernant
	int tab_cara[NB_CARACTERISTIQUES];
	for (int i = 0; i < nb_objets; i++){
            //Récupération de la taille du nom et du nom de l'objet.
            char *nom_objet = map_get_name(i);
            int taille_nom = strlen(nom_objet);
                
            write(map, &taille_nom, sizeof(int));
            write(map, nom_objet, taille_nom * sizeof(char));
                
            tab_cara[0] = map_get_frames(i);
            tab_cara[1] = map_get_solidity(i);
            tab_cara[2] = map_is_destructible(i);
            tab_cara[3] = map_is_collectible(i);
            tab_cara[4] = map_is_generator(i);

            write(map, &tab_cara, NB_CARACTERISTIQUES * sizeof(int));
        }
	close(map);
        fprintf(stdout, "Game saved successfully !\n");
}
	


void map_load (char *filename){
    
        //Numéro d'erreur
        errno = 0;

	int largeur = 0;
	int hauteur = 0;
	int nb_objets = 0;

	int objet = 0;

        //Ouverture du fichier
	int map = open(filename, O_RDONLY);
        if (map == -1){
            fprintf(stderr, "Error creating the save file. %s", strerror(errno));
            exit(errno);
        }

	//Lecture des informations de base : largeur, hauteur et nb_objets
	read(map, &largeur, sizeof(int));
	read(map, &hauteur, sizeof(int));
	read(map, &nb_objets, sizeof(int));
        
	map_allocate(largeur, hauteur);

	//Parcours de la matrice et placement des objets sur la carte
	for (int y = 0; y < hauteur; y++){
		for (int x = 0; x < largeur; x++){
			read(map, &objet, sizeof(int));
                        if (objet != -1)
                            map_set(x, y, objet);
                }
	}

	int taille_nom = 0;
	int nb_sprites = 0;
	int collectible = 0;
	int destructible = 0;
	int generator = 0;
	int solid = 0;

	map_object_begin(nb_objets);
        
        //récupération dans le fichier de chaque objet.
	for (int i = 0; i < nb_objets; i++){
		read(map, &taille_nom, sizeof(int));
		
                char nom_objet[taille_nom];
                char caractere_nom;
                
                //Récupération du nom de l'objet caractère par caractère.
                for (int i = 0; i< taille_nom; i++){
                  read(map, &caractere_nom, sizeof(char));
                  nom_objet[i] = caractere_nom;
                }
                //Lecture des caractéritiques dans le fichier
		read(map, &nb_sprites, sizeof(int));
		read(map, &solid, sizeof(int));
		read(map, &destructible, sizeof(int));
		read(map, &collectible, sizeof(int));
		read(map, &generator, sizeof(int));
                
                //récupération des flags uniquement à 1
                int flags = generer_flags(solid, collectible, destructible, generator);

                map_object_add (nom_objet, nb_sprites, flags);
	}
        map_object_end();
}
#endif
