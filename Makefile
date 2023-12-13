all : S.out J.out

S.out : GestionnairePartie.c
	gcc -o S.out GestionnairePartie.c

J.out : Joueur.c
	gcc -o J.out Joueur.c

clean :
	rm -f *.out