#!/bin/bash

# Vérification du nombre d'arguments
if [ $# -eq 0 ]; then
    echo "Aucune valeur passée en argument."
    exit 1
fi

# Nom du fichier
filename="PartieScore.txt"

# Vérification de l'existence du fichier
if [ -f "$filename" ]; then
    echo "Le fichier $filename existe déjà."
else
    echo "Le fichier $filename n'existe pas."
    touch "$filename"
fi

echo "Manche $1" >> "$filename"
#variable pour le nombre de joueur
nbJoueur=0
# Boucle pour écrire les valeurs dans le fichier
for arg in "$@"; do
    #verifier si c'est le premier argument
    if [ $nbJoueur -eq 0 ]; then
        ((nbJoueur++))
    else
        echo "Joueur $nbJoueur : $arg" >> "$filename"
        ((nbJoueur++))
    fi
done

echo "Les valeurs ont été écrites dans le fichier $filename."
