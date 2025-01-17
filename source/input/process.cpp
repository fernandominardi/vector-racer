

// funcion para controlar los efectos de la presion de teclas (o la no presion de estas)
void processInput() {

	/***********************************
		Seccion con posibilidad de ninguna tecla presionada
	************************************/
	
	// en caso de que la nave no se mueva lateralmente se contraresta la inclinacion de la nave
	if( 
		(
			( !gKEYBOARD.specialKeyPressed(GLUT_KEY_RIGHT) and !gKEYBOARD.specialKeyPressed(GLUT_KEY_LEFT) )
			or
			( gKEYBOARD.specialKeyPressed(GLUT_KEY_RIGHT) and gKEYBOARD.specialKeyPressed(GLUT_KEY_LEFT) )
		)
		and 
		gCAR_TILT 
	){
		if( 0 < gCAR_TILT )
			gCAR_TILT -= TILT_SPEED;
		if( gCAR_TILT < 0 )
			gCAR_TILT += TILT_SPEED;
	}
	
	/****************************************************
		Seccion de tecla(s) necesariamente presionada(s)
	****************************************************/
	if( !gKEYBOARD.keyPressed() )
		return;
	
	// activacion/desactivacion del modo debug
	if( gKEYBOARD.asciiKeyPressed('q') ){
		gKEYBOARD.removeAsciiKey('q');
		gDEBUG_MODE = !gDEBUG_MODE;
		if( gDEBUG_MODE ){
			gSOUND_ENGINE->play2D("../media/done-hacked.ogg");
		}else{
			// al salir del modo debug (o cheat) se resetan las variables de juego, incluyendo la camara
			if( gIN_GAME_STATE==PLAYING ){
				gCAM_POS.setXYZ(0,0,0);
				gCAM_DIR.setXYZ(0,0,1);
				gCAR_SPEED = 0.2;
				glutTimerFunc(100,scorePP,100);
			}
		}
	}
	
	switch( gGENERAL_STATE ){
		//==================================
		// controles para el menu principal
		case MAIN_MENU:{
			if( gKEYBOARD.asciiKeyPressed(ESC) ){
				gGENERAL_STATE = EXIT;
			}
			if( gKEYBOARD.asciiKeyPressed(ENTER)){
				gKEYBOARD.removeAsciiKey(ENTER);
				gGENERAL_STATE = IN_GAME;
				gSOUND_ENGINE->play2D("../media/next-menu.wav");
				gIN_GAME_STATE = INSTRUCTIONS;
			}
		}break;
		
		//==================================
		// controles in game
		case IN_GAME:{
			switch( gIN_GAME_STATE ){
				//==================================
				// seccion de instrucciones
				case INSTRUCTIONS:{
					if( gKEYBOARD.asciiKeyPressed(ENTER) ){
						gKEYBOARD.removeAsciiKey(ENTER);
						gSOUND_ENGINE->play2D("../media/next-menu.wav");
						gIN_GAME_STATE = GAME_INIT;
					}
					if( gKEYBOARD.asciiKeyPressed(ESC) ){
						gKEYBOARD.removeAsciiKey(ESC);
						gGENERAL_STATE = EXIT;
					}
				}break;
				
				//==================================
				// controles del juego en progreso
				case PLAYING:{
					if( gDEBUG_MODE ){
						// ===== mover camara ===========
						
						if( gKEYBOARD.asciiKeyPressed('w') ){
							gCAM_POS += gCAM_DIR*gCAM_TRASNLATE_SPEED;
						}
						if( gKEYBOARD.asciiKeyPressed('a') ){
							Vector3D auxCamDir(gCAM_DIR);
							
							double angleXZ = auxCamDir.getAngleXZ()-HALF_PI;
							auxCamDir.setAngleXZ( HALF_PI );
							auxCamDir.setAngleYZ( HALF_PI );
							auxCamDir.setAngleXZ( angleXZ );
							
							gCAM_POS += auxCamDir*gCAM_TRASNLATE_SPEED;
						}
						
						if( gKEYBOARD.asciiKeyPressed('s') ){
							gCAM_POS -= gCAM_DIR*gCAM_TRASNLATE_SPEED;
						}
						
						if( gKEYBOARD.asciiKeyPressed('d') ){
							Vector3D auxCamDir(gCAM_DIR);
							
							double angleXZ = auxCamDir.getAngleXZ()+HALF_PI;
							auxCamDir.setAngleXZ( HALF_PI );
							auxCamDir.setAngleYZ( HALF_PI );
							auxCamDir.setAngleXZ( angleXZ );
							
							gCAM_POS += auxCamDir*gCAM_TRASNLATE_SPEED;
						}
						
						if( gKEYBOARD.asciiKeyPressed('r') ){
							gCAM_POS.setXYZ(0,0,0);
							gCAM_DIR.setXYZ(0,0,1);
						}
						
						// ===== end mover camara =======
						
						if( gKEYBOARD.asciiKeyPressed('x') ){
							gKEYBOARD.removeAsciiKey('x');
							if(gCAR_SPEED > 0)
								gCAR_SPEED = 0.0;
							else
								gCAR_SPEED = 0.2;
						}
					}
					
					if( gKEYBOARD.specialKeyPressed(GLUT_KEY_LEFT) and !gKEYBOARD.specialKeyPressed(GLUT_KEY_RIGHT) ){
						if( fabs(gCAR_TILT-TILT_SPEED) < 45 )
							gCAR_TILT -= TILT_SPEED;
						gCAR_POS.setX( gCAR_POS.getX()+gCAR_LATERAL_SPEED);
						if(gCAR_POS.getX() > ROAD_LIMIT-CAR_WIDTH/2.0)
							gCAR_POS.setX(ROAD_LIMIT-CAR_WIDTH/2.0);
					}
					
					if( gKEYBOARD.specialKeyPressed(GLUT_KEY_RIGHT) and !gKEYBOARD.specialKeyPressed(GLUT_KEY_LEFT) ){
						if( fabs(gCAR_TILT+TILT_SPEED) < 45 )
							gCAR_TILT += TILT_SPEED;
						gCAR_POS.setX( gCAR_POS.getX()-gCAR_LATERAL_SPEED);
						if(gCAR_POS.getX() < -ROAD_LIMIT+CAR_WIDTH/2.0)
							gCAR_POS.setX(-ROAD_LIMIT+CAR_WIDTH/2.0);
					}
					
					if( gKEYBOARD.asciiKeyPressed(ESC) ){
						gKEYBOARD.removeAsciiKey(ESC);
						gIN_GAME_STATE = GAME_OVER_INIT;
					}
					
				}break;
				
				//==================================
				// controles del estado game over
				case GAME_OVER:{
					if( gKEYBOARD.asciiKeyPressed(ENTER) ){
						gKEYBOARD.removeAsciiKey(ENTER);
						gSOUND_ENGINE->play2D("../media/next-menu.wav");
						gIN_GAME_STATE = GAME_OVER_END;
					}
					if( gKEYBOARD.asciiKeyPressed(ESC) ){
						gKEYBOARD.removeAsciiKey(ESC);
						gGENERAL_STATE = EXIT;
					}
					if( gSCORE_STATE != NONE ){
						if( gKEYBOARD.asciiKeyPressed(BACKSPACE) ){
							gKEYBOARD.removeAsciiKey(BACKSPACE);
							if (gPLAYER_NAME.size () > 0)
								gPLAYER_NAME.resize(gPLAYER_NAME.size()-1);
						}
						if( gKEYBOARD.asciiKeyPressed() ){
							unsigned char key = gKEYBOARD.getNextAsciiKey();
							if( isprint(key) )
								gPLAYER_NAME += key;
						}
					}
				}break;
				
				// casos sin control de teclado
				case GAME_INIT:{}break;
				case GAME_OVER_INIT:{}break;
				case GAME_OVER_END:{} break;
			}
		}break;
		
		// en los casos siguientes no se realiza control de teclas presionadas
		case EXIT:{}break;
		
	}
	
}





































