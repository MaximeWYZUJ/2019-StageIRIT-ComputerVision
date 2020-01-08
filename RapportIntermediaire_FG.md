# FENETRE GLISSANTE

## Introduction : Déroulement de l'algorithme
On dispose de deux images : un pattern issu d'une base de données et une image dans laquelle on veut savoir si l'on peut trouver ce pattern, et si oui à quel endroit. Pour faire cela, on crée une fenêtre que l'on va déplacer dans l'image. Après chaque déplacement, on mesure un score de corrélation entre la région de l'image à l'intérieur de la fenêtre, et le pattern que l'on veut identifier. Une fois que l'on a calculé tous les scores, on les trie et on ne garde que ceux dépassant un certain seuil.
Les coordonnées des fenêtres associées à ces scores sont les endroits où se trouvent le pattern.

Plusieurs questions se posent. Concernant la fenêtre, comment choisir sa taille et est-ce pertinant de parcourir toute l'image ? Comment se calcule le score de corrélation et quels critères permettent de déterminer la seuil permettant distinguer les bons et les mauvais scores ?


## Définition de la fenêtre glissante
Pour trouver le pattern dans l'image, on veut aussi tirer partie du fait que, dans la plupart des jeux 2D, les éléments sont agencés sur une grille (*tilemap*) dont chaque case (tuile) est susceptible de contenir le pattern. Cette information nous permet de chercher le pattern à des emplacements prédéfinis, plutôt que de parcourir toute l'image pixel par pixel. En revanche, il nous faut connaître les coordonnées de la grille par rapport à l'image (*offsets*) et les dimensions d'une tuile, comme indiqué sur l'image ci-dessous. Evidemment, le gain de performances est énorme.

![tile map](https://imagizer.imageshack.com/img924/2880/zUgMrw.png)

Même si c'est souvent la même en pratique, en théorie les dimensions des éléments du jeu sont différentes de celles d'une tuile (on peut imaginer qu'un objet soit de dimension deux fois plus petite que la tuile et qu'il puisse se trouver aux quatre coins de celle-ci; en le cherchant au milieu on ne le trouvera jamais). De plus, les patterns de la base de données ne conservent pas leurs dimensions une fois rendus dans le jeu. Pour ces deux raisons, il nous faut l'information de *scaling* entre la base de données et l'image afin de redimensionner notre pattern aux dimensions de ce même pattern dans l'image. Ceci amène quelques erreurs d'interpolation qui restent négligeables.

Sur des cas simples où le pattern occupe toute la tuile, il nous faut donc 2 informations a priori : l'*offset* de la grille par rapport aux bords de l'image ainsi que le ratio entre le pattern de notre base de données et le pattern dans l'image. Dans des cas plus compliqués, il nous faut également les dimensions d'une tuile dans l'image. Pour l'instant, je n'ai traité que des cas simples.


## Mesure de la corrélation
Une fois que l'on a positionné notre fenêtre sur une case de la grille, on peut comparer la sous-image correspondant à cette case avec notre pattern aux bonnes dimensions.

On commence donc pas créer un masque à partir du canal alpha du pattern. Ce masque nous permet de mesurer la corrélation uniquement entre les pixels utiles de notre pattern (ceux où il y a de la couleur). On peut ensuite mesurer la corrélation entre ces pixels utiles à l'aide de différentes mesures : L1, L2 ou ZNCC par exemple.

Les figures ci-dessous illustrent l'influence, sur ces mesures, du décalage entre la fenêtre et le pattern. On voit que si l'on applique la fenêtre précisémment où se trouve le pattern, les mesures arrivent à se distinguer. En revanche, le moindre pixel de décalage entraine de grosses erreurs de mesure. Comme on peut le voir sur la dernière figure, c'est la mesure L2 qui est la moins sensible aux décalages puisque entre 2 et 4 pixels de décalage, on peut toujours considérer le score comme correct.

![L1 3D](https://imagizer.imageshack.com/img922/2539/uggeLf.jpg)
> Mesure L1
<!--- --->
![L2 3D](https://imagizer.imageshack.com/img923/8302/AZqWTA.jpg)
> Mesure L2
<!--- --->
![ZNCC 3D](https://imagizer.imageshack.com/img923/5179/hPdb6j.jpg)
> Mesure ZNCC
<!--- --->
![Decalage](https://imagizer.imageshack.com/img923/4088/ycsOqs.jpg)
> Mesures en fonction du nombre de pixels de décalage
<!--- --->


## Résultats et pistes d'amélioration
Les résultats obtenus sur des exemples simples où les éléments sont de même dimension que la tuile sont très satisfaisants.

Cependant, la qualité des résultats est très sensible à l'*offset* de la grille. J'ai remarqué que les éléments à trouver ne sont pas toujours au centre de la tuile, mais parfois décalés de quelques pixels. Or pour le moment je ne cherche qu'au centre de la tuile. Un moyen simple de palier à ce problème est de modifier ma base de données de patterns de sorte qu'ils soient décalés de la même manière que dans le jeu.

![Spice berry opti](https://imagizer.imageshack.com/img922/3246/5FjI1n.png)
![Spice berry decalage 2](https://imagizer.imageshack.com/img921/1762/bFWJ6H.png)
> Détection des baies épicées avec un offset optimal, puis avec un offset décalé de 2 pixels par rapport à l'offset optimal.
<!--- --->

On peut aussi filtrer simplement les cases qui n'ont rien à voir avec le pattern. En comparant les histogrammes du pattern et de chaque case de la grille, on peut seuiller afin de traiter uniquement les cases contenant plus ou moins les bonnes couleurs. En plus, cette méthode n'est quasiment pas impactée par le décalage.



# FENETRE GLISSANTE SUR UNE PYRAMIDE D'IMAGES

## Introduction

On vient de voir que le point faible de l'algorithme précédent est sa sensibilité vis à vis de l'offset choisi initialement. On peut supprimer ce problème en décalant la fenêtre pixel par pixel. Néanmoins, le temps de calcul ne serait pas raisonnable (environ 37h pour une image 1920x1080 et un pattern 64x64 avec la norme L1). On se sert donc d'une pyramide d'images pour cibler des régions pouvant contenir les patterns recherchés.

On commence par faire glisser notre fenêtre pixel par pixel dans une image de faible résolution afin d'identifier des régions pouvant correspondre au pattern voulu. Puis on transpose ces régions dans une image de meilleure résolution où l'on affine la position du pattern. Une fois que l'on a remonté tous les étages de notre pyramide d'images, on a identifié les positions des fenêtres pouvant contenir le pattern. Pour vérifier si c'est le cas, il suffit de seuiller les mesures de corrélation obtenues pour chacune de ces fenêtres.


# Comment créer la pyramide d'images ?

C'est une liste d'images parcourue de la plus faible résolution à la plus forte. Cependant, il nous faut définir le nombre d'éléments de cette liste et la façon dont on baisse la résolution de l'image de départ.

Le choix du nombre d'étages de la pyramide est important puisqu'il influera sur la résolution de la plus petite image. Or c'est l'image la plus déterminante dans la détection de patterns puisque c'est à partir de cette image que l'on initialisera les zones où rechercher le pattern. Ainsi, si l'image n'est pas d'assez bonne qualité, on cherchera le pattern dans les mauvaises zones. A l'inverse, si l'on choisit une image de trop forte résolution, le temps de calcul sera trop long.

Pour réduire l'image, on peut avoir une pyramide dont la i-ème image est l'image d'origine multipliée par un facteur i/N (avec N le nombre d'étages). La taille de chaque zone de recherche augmente alors de façon linéaire. On peut sinon diviser par deux la résolution de nos images à chaque étage pour arriver plus rapidement à une image de faible résolution. La taille des zones de recherche augmente donc de manière exponentielle.

De manière empirique, j'estime que la résolution minimale pour obtenir des résultats corrects est de 1/4 de celle d'origine.