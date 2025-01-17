
#include "objLoader.hpp"
//función de carga de objetos (parser)
int objLoader::load(const char* filename){
	//abrimos el archivo del modelo
	std::ifstream in(filename);	
	//if not opened, exit with -1
	if(!in.is_open()){
		std::cout << "No se pudo abrir el archivo de modelo " << filename << std::endl;
		return -1;
	}
	//buffer de lectura
	char buf[256];
	//material actual
	int curMat = 0;
	//leemos todo el archivo y lo cargamos a coord
	while(!in.eof()){
		in.getline(buf,256);
		coord.push_back(new std::string(buf));
	}
	//iteramos cada elemento de coord 
	for(unsigned int i=0;i<coord.size();i++)	{
		if(coord[i]->c_str()[0]=='#')						//si es un comentario, lo ignoramos
			continue;
		else if(coord[i]->c_str()[0]=='v' && coord[i]->c_str()[1]==' '){	//encontramos la definición de un vector
			float tmpx,tmpy,tmpz;
			sscanf(coord[i]->c_str(),"v %f %f %f",&tmpx,&tmpy,&tmpz);
			vertex.push_back(new coordinate(tmpx,tmpy,tmpz));
		}else if(coord[i]->c_str()[0]=='v' && coord[i]->c_str()[1]=='n'){	//encontramos la definición de una normal
			float tmpx,tmpy,tmpz;
			sscanf(coord[i]->c_str(),"vn %f %f %f",&tmpx,&tmpy,&tmpz);
			normals.push_back(new coordinate(tmpx,tmpy,tmpz));
		}else if(coord[i]->c_str()[0]=='f'){					//encontramos la definición de una cara
											//la logica de las caras en los obj es la siguiente:
											// formato a b c d : es la lista de puntos (cada punto es un elemento coordinate)
											// formato a//v b//v c//v d//v : es la lista de puntos (cada punto es un elemento 
											// coordinate) y v es su vector normal
											//
											// en cada paso se verifica el formato de definicion de una cara y luego se 
											// extraen los datos del archivo de definición para cargarlos al stack de caras del modelo.
			
			int a,b,c,d,e;								//variables auxiliares
			if(count(coord[i]->begin(),coord[i]->end(),' ')==3){			//es un triangulo
				if(coord[i]->find("//") != std::string::npos){			// formato separado por // (vectores y normal)
					sscanf(coord[i]->c_str(),"f %d//%d %d//%d %d//%d",&a,&b,&c,&b,&d,&b);
					faces.push_back(new face(b,a,c,d,0,0,0,curMat));
				}else if(coord[i]->find("/") != std::string::npos){		//formato separado por / (vectores normal y textura)
					int t[3];
					sscanf(coord[i]->c_str(),"f %d/%d/%d %d/%d/%d %d/%d/%d",&a,&t[0],&b,&c,&t[1],&b,&d,&t[2],&b);
					faces.push_back(new face(b,a,c,d,t[0],t[1],t[2],curMat));	
				}else{								//formato sin separacion (solo vectores)
					sscanf(coord[i]->c_str(),"f %d %d %d",&a,&b,&c);
					faces.push_back(new face(-1,a,b,c,0,0,0,curMat));
				}
			}else if(count(coord[i]->begin(),coord[i]->end(),' ')==4){		//se hace especifico por las dudas, de tal forma que un archivo con mal formato no genere segmentation fault
				if(coord[i]->find("//") != std::string::npos){			// formato separado por // (vectores y normal)
					sscanf(coord[i]->c_str(),"f %d//%d %d//%d %d//%d %d//%d",&a,&b,&c,&b,&d,&b,&e,&b);
					faces.push_back(new face(b,a,c,d,e,0,0,0,0,curMat));
				}else if(coord[i]->find("/") != std::string::npos){		//formato separado por / (vectores normal y textura)
					int t[4];
					sscanf(coord[i]->c_str(),"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",&a,&t[0],&b,&c,&t[1],&b,&d,&t[2],&b,&e,&t[3],&b);
					faces.push_back(new face(b,a,c,d,e,t[0],t[1],t[2],t[3],curMat));	
				}else{								//formato sin separacion (solo vectores)
					sscanf(coord[i]->c_str(),"f %d %d %d %d",&a,&b,&c,&d);
					faces.push_back(new face(-1,a,b,c,d,0,0,0,0,curMat));
				}
			}
		}else if(coord[i]->c_str()[0] == 'u' && coord[i]->c_str()[1] == 's' && coord[i]->c_str()[2] == 'e'){	//define un cambio en el material 
			char tmp[200];
			sscanf(coord[i]->c_str(),"usemtl %s",tmp);							//obtenemos el nombre del material
			for(unsigned int a = 0 ; a < materials.size(); a++){						//recorremos la lista de materiales que ya tenemos
				if(strcmp(materials[a]->name.c_str(),tmp)==0){						//cuando encontramos el material, establecemos ese material como currentMaterial
					curMat = a;
					break;
				}
			}
		}else if(coord[i]->c_str()[0] == 'm' && coord[i]->c_str()[1] == 't' && coord[i]->c_str()[2] == 'l' && coord[i]->c_str()[3] == 'l'){	//comando mtllib, para parsear la biblioteca de materiales
																			//dentro de este if, debemos leer el archivo de materiales
																			//definido por el archivo de definición del modelo.

			char fileN[200];														//buffer para el nombre del archivo
			sscanf(coord[i]->c_str(),"mtllib %s",fileN);											//extraemos el nombre del archivo
			std::ifstream mtlin(fileN);													//tratamos de abrir el archivo
			if(!mtlin.is_open()){														//si no pudimos abrir el archivo retoramos un error
				std::cout << "No se puede abrir el archivo de materiales" << fileN << std::endl;
				clean();
				return -1;
			}
																			//pudimos abrir el archivo =D
			isMaterial = true;														//hay que considerar materiales en el dibujado
			std::vector<std::string> tmp;													//cada linea del archivo de materiales que vamos a parsear
			char buf[200];															//buffer de lectura
			while(!mtlin.eof()){														//leemos todo el archivo y cargamos el contenido en el buffer de lectura
				mtlin.getline(buf,200);
				tmp.push_back(buf);
			}
			char name[200];															//nombre del material
			char filename[200];														//nombre del archivo de la textura
			float amb[3],dif[3],spec[3], alpha, ns, ni;											//informacion de la respuesta del material a la luz
			int illum;															//factor de luminicidad
			unsigned int texture;														//indice de textura
			bool isMat = false;														//bandera que indica si tenemos cargado algun material en estas variables
			strcpy(filename,"\0");														//vaciamos el buffer del nombre de la textura
			for(unsigned int j=0; j < tmp.size(); j++){											//recorremos todo el buffer de lectura
				if(tmp[j][0] == '#')														//es un comentario, ignoramos sin preguntar el resto
					continue;
				if(tmp[j][0] == 'n' && tmp[j][1] == 'e' && tmp[j][2] == 'w'){									//nuevo material
					if(isMat){															//si tenemos un nuevo material, cargamos al stack de materiales
						if(strcmp(filename,"\0") != 0){													//tenemos textura?
							materials.push_back(new material(name, alpha, ns, ni, dif, amb, spec, illum, texture));
							strcpy(filename,"\0");
						}else{																//sin textura
							materials.push_back(new material(name, alpha, ns, ni, dif, amb, spec, illum, -1));
						}
					}
					isMat = false;															//estamos llenando variables para un nuevo material
					sscanf(tmp[j].c_str(),"newmtl %s",name);											//obtenemos el nombre del nuevo material
				}else if(tmp[j][0] == 'N' && tmp[j][1] == 's'){												//shininess
					sscanf(tmp[j].c_str(),"Ns %f",&ns);
					isMat = true;
				}else if(tmp[j][0] == 'K' && tmp[j][1] == 'a'){												//valores de la luz ambiental
					sscanf(tmp[j].c_str(),"Ka %f %f %f",&amb[0],&amb[1],&amb[2]);
					isMat = true;
				}else if(tmp[j][0] == 'K' && tmp[j][1] == 'd'){												//valores de luz difusa
					sscanf(tmp[j].c_str(),"Kd %f %f %f",&dif[0],&dif[1],&dif[2]);
					isMat = true;
				}else if(tmp[j][0] == 'K' && tmp[j][1] == 's'){												//valores de luz especular
					sscanf(tmp[j].c_str(),"Ks %f %f %f",&spec[0],&spec[1],&spec[2]);
					isMat = true;
				}else if(tmp[j][0] == 'N' && tmp[j][1] == 'i'){												//indice de refraccion
					sscanf(tmp[j].c_str(),"Ni %f",&ni);
					isMat = true;
				}else if(tmp[j][0] == 'd' && tmp[j][1] == ' '){												//cantidad de transparencia
					sscanf(tmp[j].c_str(),"d %f",&alpha);
					isMat = true;
				}else if(tmp[j][0] == 'i' && tmp[j][1] == 'l'){												//modelo de iluminacion
					sscanf(tmp[j].c_str(),"illum %d",&illum);
					isMat = true;
				}else if(tmp[j][0] == 'm' && tmp[j][1] == 'a'){												//archivo de textura
					sscanf(tmp[j].c_str(),"map_Kd %s",filename);
					texture = loadTexture(filename);
					isMat = true;
				}
			}
			if(isMat){																//tenemos que verificar "manualmente" el ultimo material, ya que
																				//luego del mismo no tenemos un comando newmtl que nos
																				//indique del fin del material anterior
			
				if(strcmp(filename,"\0") != 0){													//si el buffer del nombre del archivo de textura tiene algo,
																				//significa que tenemos el texture mapping 
																				//correspondiente
					materials.push_back(new material(name, alpha, ns, ni, dif, amb, spec, illum, texture));
				}else{																//en caso contrario, ignoramos la textura
					materials.push_back(new material(name, alpha, ns, ni, dif, amb, spec, illum, -1));
				}	
			}
		}else if(coord[i]->c_str()[0] == 'v' && coord[i]->c_str()[1] == 't'){										//coordenadas de textura (archivo .obj)
			float u,v;
			sscanf(coord[i]->c_str(),"vt %f %f",&u,&v);
			textureCoordinate.push_back(new textCoord(u,v)); 
			isTexture = true;															//consideramos que tiene textura
		}
	}
	if(materials.size() == 0)															//si por algun motivo no tenemos materiales cargados no los consideramos en el dibudado
		isMaterial = false;
	else			
		isMaterial = true;
			
	//dibujado
	int num;																	//numero para la lista de generacion de OpenGL
	num = glGenLists(1);																//generamos la lista
	glNewList(num, GL_COMPILE);															//inicio de la lista asociado al numero recien creado
	int last = -1;																	//se inicializa en -1 para forzar a que se utilice el primer material, ya que el indice -1 no existe
	for(unsigned int i=0; i<faces.size(); i++){													//recorremos cada cara para el dibujo
		
		if(last != faces[i]->mat && isMaterial){													//si debemos considerar materiales y hay un cambio de material, configuramos el nuevo material
			float diffuse[]={materials[faces[i]->mat]->dif[0],materials[faces[i]->mat]->dif[1],materials[faces[i]->mat]->dif[2],1.0};
			float ambient[]={materials[faces[i]->mat]->amb[0],materials[faces[i]->mat]->amb[1],materials[faces[i]->mat]->amb[2],1.0};
			float specular[]={materials[faces[i]->mat]->spec[0],materials[faces[i]->mat]->spec[1],materials[faces[i]->mat]->spec[2],1.0};
                        
			
			
			glMaterialfv(GL_FRONT,GL_DIFFUSE,diffuse);
			glMaterialfv(GL_FRONT,GL_AMBIENT,ambient);
			glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
			glMaterialf(GL_FRONT,GL_SHININESS,materials[faces[i]->mat]->ns);
			last=faces[i]->mat;																//actualizamos la variable del ultimo material usado para el siguiente cambio
			
			if(materials[faces[i]->mat]->texture == -1){													//si no contamos con textura, deshabilitamos la textura a OpenGL para exponer el material
				glDisable(GL_TEXTURE_2D);
			}else{																		//si tenemos textura, habilitamos a OpenGL el manejo de texturas y hacemos el binding al archivo correspondiente
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D,materials[faces[i]->mat]->texture);
			}
			
		}
		
		if(faces[i]->four)      															//si lo que tenemos que dibujar es un GL_QUAD
		{
			glBegin(GL_QUADS);
			if(isNormals)   																//si tenemos normales definidas
				glNormal3f(normals[faces[i]->faceNormal-1]->x,normals[faces[i]->faceNormal-1]->y,normals[faces[i]->faceNormal-1]->z);    			//las utilizamos

			if(isTexture && materials[faces[i]->mat]->texture!=-1)  											//si hay textura
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[0]-1]->u,textureCoordinate[faces[i]->texCoord[0]-1]->v);      				//mapeamos la textura al modelo

			glVertex3f(vertex[faces[i]->faces[0]-1]->x,vertex[faces[i]->faces[0]-1]->y,vertex[faces[i]->faces[0]-1]->z);					//dibujamos la primera coordenada
																					//el proceso se repite para las siguientes coordenadas

			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[1]-1]->u,textureCoordinate[faces[i]->texCoord[1]-1]->v);

			glVertex3f(vertex[faces[i]->faces[1]-1]->x,vertex[faces[i]->faces[1]-1]->y,vertex[faces[i]->faces[1]-1]->z);					//segunda

			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[2]-1]->u,textureCoordinate[faces[i]->texCoord[2]-1]->v);

			glVertex3f(vertex[faces[i]->faces[2]-1]->x,vertex[faces[i]->faces[2]-1]->y,vertex[faces[i]->faces[2]-1]->z);					//tercera

			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[3]-1]->u,textureCoordinate[faces[i]->texCoord[3]-1]->v);

			glVertex3f(vertex[faces[i]->faces[3]-1]->x,vertex[faces[i]->faces[3]-1]->y,vertex[faces[i]->faces[3]-1]->z);					//cuarta
			glEnd();
		}else{																		//en otro caso, lo que tenemos que utilizar es GL_TRIANGLES
			glBegin(GL_TRIANGLES);
			if(isNormals)
				glNormal3f(normals[faces[i]->faceNormal-1]->x,normals[faces[i]->faceNormal-1]->y,normals[faces[i]->faceNormal-1]->z);

			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[0]-1]->u,textureCoordinate[faces[i]->texCoord[0]-1]->v);


			glVertex3f(vertex[faces[i]->faces[0]-1]->x,vertex[faces[i]->faces[0]-1]->y,vertex[faces[i]->faces[0]-1]->z);					//primera coordenada

			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[1]-1]->u,textureCoordinate[faces[i]->texCoord[1]-1]->v);

			glVertex3f(vertex[faces[i]->faces[1]-1]->x,vertex[faces[i]->faces[1]-1]->y,vertex[faces[i]->faces[1]-1]->z);					//segunda


			if(isTexture && materials[faces[i]->mat]->texture!=-1)
				glTexCoord2f(textureCoordinate[faces[i]->texCoord[2]-1]->u,textureCoordinate[faces[i]->texCoord[2]-1]->v);

			glVertex3f(vertex[faces[i]->faces[2]-1]->x,vertex[faces[i]->faces[2]-1]->y,vertex[faces[i]->faces[2]-1]->z);					//tercera
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
	glEndList();																	//cerramos la lista																
	clean();
	lists.push_back(num);																//agregamos el modelos al stack
	return num;																	//retornamos el id del modelo
}

//libera la memoria del cargador de objetos
void objLoader::clean(){
	for(unsigned int i=0;i<coord.size();i++)
		delete coord[i];
	for(unsigned int i=0;i<faces.size();i++)
		delete faces[i];
	for(unsigned int i=0;i<normals.size();i++)
		delete normals[i];
	for(unsigned int i=0;i<vertex.size();i++)
		delete vertex[i];
	for(unsigned int i=0;i<materials.size();i++)
		delete materials[i];
	for(unsigned int i=0;i<textureCoordinate.size();i++)
		delete textureCoordinate[i];
	coord.clear();
	faces.clear();
	normals.clear();
	vertex.clear();
	materials.clear();
	textureCoordinate.clear();
}

//constructor
objLoader::objLoader(){
	isMaterial = false;
	isNormals = false;
	isTexture = false;
}

//destructor
objLoader::~objLoader(){
	for(std::vector<unsigned int>::const_iterator it=texture.begin(); it!=texture.end(); it++){
		glDeleteTextures(1,&(*it));
	}
	for(std::vector<unsigned int>::const_iterator it=lists.begin(); it!=lists.end(); it++){
		glDeleteLists(*it,1);
	}
}

//cargador de texturas para el model loader
unsigned int objLoader::loadTexture(const char * filename){
	//identificador de la textura
	unsigned int num;	
	//generamos la textura, creando el identificador
	glGenTextures(1,&num);
	//cargamos la imagen del archivo
	Image* img = loadBMP(filename);
	//hacemos el binding correspondiente
	glBindTexture(GL_TEXTURE_2D,num);
	//seteamos parametros a OpenGL para el archivo cargado
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->pixels);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//descargamos la textura de la memoria del cargador de BMP
	delete img;
	//pusheamos la textura al stack
	texture.push_back(num);
	//retornamos el identificador
	return num;
}

//cargador de texturas independientes del modelo (no se agrega la textura al stack)
unsigned int objLoader::loadSingleTexture(const char * filename){
	unsigned int num;
	glGenTextures(1,&num);
	Image* img = loadBMP(filename);
	glBindTexture(GL_TEXTURE_2D,num);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->pixels);
	glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	delete img;
	return num;
}
