/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  BtOgre test application, main file.
 *
 *        Version:  1.0
 *        Created:  01/14/2009 05:48:31 PM
 *
 *         Author:  Nikhilesh (nikki)
 *
 * =====================================================================================
 */

#include "OgreApplicationContext.h"
#include <OgreCamera.h>
#include <OgreResourceGroupManager.h>
#include <OgreFrameListener.h>
#include <OgreViewport.h>
#include <OgreTrays.h>
#include "OgreCameraMan.h"
#include "BtOgre.h"
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <Windows.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include "conio.h"

using namespace Ogre;
using namespace std;

/*
 * =====================================================================================
 *        Class:  BtOgreTestApplication
 *  Description:  Derives from ExampleApplication and overrides stuff.
 * =====================================================================================
 */

class BtOgreTestApplication : public OgreBites::ApplicationContext, public OgreBites::InputListener, public OgreBites::TrayListener
{
    std::unique_ptr<BtOgre::DynamicsWorld> mDynWorld;

	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;

	Ogre::SceneNode *mNinjaNode;
	Ogre::Entity *mNinjaEntity;
	Ogre::SceneNode* mNinja2Node;
	Ogre::Entity* mNinja2Entity;
	Ogre::SceneNode* mOgroNode;
	Ogre::Entity* mOgroEntity;
	Ogre::SceneNode* mOgro2Node;
	Ogre::Entity* mOgro2Entity;
	Ogre::Entity *mGroundEntity;

	Ogre::Bone* hand;

	btRigidBody* bananoR;
	btRigidBody* ogroR;
	btRigidBody* ogro2R;
	btRigidBody* mapaR;

	bool turno = true;

	int vidas1 = 5;
	int vidas2 = 5;

	String nombre1 = "Mono 1";
	String nombre2 = "Mono 2";

	OgreBites::CameraMan *mCamMan;

	bool mDebugOn;
    std::unique_ptr<BtOgre::DebugDrawer> mDbgDraw;

	struct OgroContactResultCallback : public btCollisionWorld::ContactResultCallback {

		OgroContactResultCallback(BtOgreTestApplication* ptr) : context(ptr) {}

		btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap,
			int partId0,
			int index0,
			const btCollisionObjectWrapper* colObj1Wrap,
			int partId1,
			int index1)
		{			
			if (context->turno) {
				context->retirarOgro(false);
				context->mostrarVidas(false);
				context->validarGanador();
			}			
			_cprintf("\n-------------------GOLPEO 1------------------------");
			return 0;
		}

		BtOgreTestApplication* context;

	};

	struct Ogro2ContactResultCallback : public btCollisionWorld::ContactResultCallback {

		Ogro2ContactResultCallback(BtOgreTestApplication* ptr) : context(ptr) {}

		btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap,
			int partId0,
			int index0,
			const btCollisionObjectWrapper* colObj1Wrap,
			int partId1,
			int index1)
		{
			if (!context->turno) {
				context->retirarOgro(true);
				context->mostrarVidas(true);
				context->validarGanador();
			}			
			_cprintf("\n-------------------GOLPEO 2------------------------");
			return 0;
		}

		BtOgreTestApplication* context;

	};

	struct MapaContactResultCallback : public btCollisionWorld::ContactResultCallback {

		MapaContactResultCallback(BtOgreTestApplication* ptr) : context(ptr) {}

		btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap,
			int partId0,
			int index0,
			const btCollisionObjectWrapper* colObj1Wrap,
			int partId1,
			int index1)
		{
			_cprintf("\n-------------------CAMBIOOOOOOOO------------------------");
			context->cambiarTurno();
			return 0;
		}

		BtOgreTestApplication* context;

	};

    public:
	BtOgreTestApplication() : OgreBites::ApplicationContext("BtOgre")
	{
        mDynWorld.reset(new BtOgre::DynamicsWorld(Ogre::Vector3(0,-9.8,0)));
		mDebugOn = true;
	}

	void shutdown()
	{
		OgreBites::ApplicationContext::shutdown();
        mDbgDraw->clear();
	}

	void setup(void)
	{		
	    OgreBites::ApplicationContext::setup();
	    addInputListener(this);

		srand(time(NULL));

	    mSceneMgr = getRoot()->createSceneManager();

		mSceneMgr->addRenderQueueListener(getOverlaySystem());

		//OgreBites::TrayManager* mTrayMgr = new OgreBites::TrayManager("InterfaceName", getRenderWindow(), this);
		//OgreBites::Button* b = mTrayMgr->OgreBites::TrayManager::createButton(OgreBites::TrayLocation::TL_TOPLEFT, "MyButton", "Click Me!");
		//delete mTrayMgr;

	    // register our scene with the RTSS
	    Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	    shadergen->addSceneManager(mSceneMgr);

	    // without light we would just get a black screen
        Ogre::Light* light = mSceneMgr->createLight("MainLight");
        Ogre::SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        lightNode->setPosition(0, 10, 15);
        lightNode->attachObject(light);

	    // create the camera
	    mCamera = mSceneMgr->createCamera("myCam");
	    mCamera->setAutoAspectRatio(true);

	    Ogre::SceneNode* camnode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	    camnode->attachObject(mCamera);

	    // and tell it to render into the main window
	    getRenderWindow()->addViewport(mCamera);

	    mCamMan = new OgreBites::CameraMan(camnode);
	    mCamMan->setStyle(OgreBites::CS_ORBIT);
	    mCamMan->setYawPitchDist(Ogre::Degree(45), Ogre::Degree(45), 20);
	    addInputListener(mCamMan);

	    //Some normal stuff.
	    mSceneMgr->setAmbientLight(ColourValue(0.7,0.7,0.7));

	    mCamera->setNearClipDistance(0.05);
	    LogManager::getSingleton().setLogDetail(LL_BOREME);

	    //----------------------------------------------------------
	    // Debug drawing!
	    //----------------------------------------------------------

	    mDbgDraw.reset(new BtOgre::DebugDrawer(mSceneMgr->getRootSceneNode(), mDynWorld->getBtWorld()));

	    //----------------------------------------------------------
	    // Ninja!
	    //----------------------------------------------------------		

		//Create Ogre stuff.
	    mNinjaEntity = mSceneMgr->createEntity("ninjaEntity", "Player.mesh");
	    mNinjaNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(0,10,0));
	    mNinjaNode->attachObject(mNinjaEntity);

		/*mNinja2Entity = mSceneMgr->createEntity("ninja2Entity", "Player.mesh");
		mNinja2Node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(10, 10, 10));
		mNinja2Node->attachObject(mNinja2Entity);*/

		mOgroEntity = mSceneMgr->createEntity("ogroEntity", "Sinbad.mesh");
		mOgroNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(10, 5, 10));
		mOgroNode->attachObject(mOgroEntity);

		mOgroNode->scale(Ogre::Vector3(0.4, 0.4, 0.4));

		mOgro2Entity = mSceneMgr->createEntity("ogro2Entity", "Sinbad.mesh");
		mOgro2Node = mSceneMgr->getRootSceneNode()->createChildSceneNode(Vector3(-10, 5, -10));
		mOgro2Node->attachObject(mOgro2Entity);

		mOgro2Node->scale(Ogre::Vector3(0.4, 0.4, 0.4));

		mOgroNode->rotate(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3(0, 1, 0)), Ogre::Node::TransformSpace::TS_WORLD);

		Ogre::SkeletonInstance* skl = mOgro2Entity->getSkeleton();

		/*_cprintf("Bones: \n");

		for (int i = 0; i < skl->getNumBones(); i++)
		{
			_cprintf("%s \n", skl->getBone(i)->getName());
		}*/
			
		/*hand = skl->getBone("Hand.R");
		hand->setManuallyControlled(true);*/				

		ubicarOgros();
		cambiarTurno();

	    //Create the Body.
        //this->bananoR = mDynWorld->addRigidBody(5, mNinjaEntity, BtOgre::CT_SPHERE);
		//mDynWorld->addRigidBody(5, mNinja2Entity, BtOgre::CT_SPHERE);
		ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
		ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);		

		//this->bananoR->setActivationState(0);

		//btCollisionObject* banano = mDynWorld->getBtWorld()->getCollisionObjectArray()[0]->get;

	    //----------------------------------------------------------
	    // Ground!
	    //----------------------------------------------------------

	    //Create Ogre stuff.
	    mGroundEntity = mSceneMgr->createEntity("groundEntity", "TestLevel_b0.mesh");
	    mSceneMgr->getRootSceneNode()->createChildSceneNode("groundNode")->attachObject(mGroundEntity);

	    //Create the Body.
        mapaR = mDynWorld->addRigidBody(0, mGroundEntity, BtOgre::CT_TRIMESH);



		_cprintf("\n-------------------------------------------------------");
		_cprintf("\nBienvenidos!!! :D \n");
		_cprintf("\n-------------------------------------------------------");
		
		_cprintf("\nIngrese el nombre del jugador 1: \n");
		_cscanf("%s", &nombre1);

		_cprintf("\nIngrese el nombre del jugador 2: \n");
		_cscanf("%s", &nombre2);

		if (nombre1 == "") {
			nombre1 = "Mono 1";
		}

		if (nombre2 == "") {
			nombre2 = "Mono 2";
		}

		int vidas = 0;
		_cprintf("\nIngrese la cantidad de vidas de los jugadores: \n");
		_cscanf("%d", &vidas);

		if (vidas != NULL && vidas != 5) {
			vidas1 = vidas;
			vidas2 = vidas;
		}

		_cprintf("\n-------------------------------------------------------");
		_cprintf("\n Que inicie el Juego !!! :D \n");
		_cprintf("\n-------------------------------------------------------");

	}

	void validarGanador() {
		if (vidas1 <= 0 || vidas2 <= 0) {

			char ch;

			_cprintf("\n-------------------------------------------------------");

			if (vidas1 <= 0) {
				_cprintf("\nFelicidades %s !!!", nombre2);		
			}
			else if(vidas2 <= 0) {
				_cprintf("\nFelicidades %s !!!", nombre1);				
			}

			_cprintf("\nHas ganado :D");
			_cprintf("\n-------------------------------------------------------");

			_cprintf("\nGracias por jugar :D\n");
			_cscanf("%c", &ch);

			// TODO: Cerrar la aplicación o implementar volver a jugar

		}
		else {
			ubicarOgros();
		}		
	}

	void mostrarVidas(bool ogro) {

		char ch;

		_cprintf("\n-------------------------------------------------------");	

		if (ogro) {
			_cprintf("\nGran Lanzamiento, punto para el %s :D", nombre2);
			vidas1 -= 1;
		}
		else {
			_cprintf("\nGran Lanzamiento, punto para el %s :D", nombre1);
			vidas2 -= 1;
		}

		cambiarTurno();

		_cprintf("\nVidas Mono 1: %d", vidas1);
		_cprintf("\nVidas Mono 2: %d", vidas2);
		_cprintf("\n-------------------------------------------------------");
		_cprintf("\nPresiona una tecla para continuar...\n");

		_cscanf("%c", &ch);

	}

	void retirarOgro(bool ogro) {

		if (ogro) {
			this->mOgro2Node->rotate(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3(1, 0, 0)), Ogre::Node::TransformSpace::TS_WORLD);
			ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);
		}
		else {
			this->mOgroNode->rotate(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3(1, 0, 0)), Ogre::Node::TransformSpace::TS_WORLD);
			ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
		}

	}

	void ubicarOgros() {

		this->mOgroNode->setPosition(Ogre::Vector3(rand() % (21) - 10, 5, rand() % (6) + 5));
		ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);

		this->mOgro2Node->setPosition(Ogre::Vector3(rand() % (21) - 10, 5, -(rand() % (6) + 5)));
		ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);

	}

	bool keyPressed(const OgreBites::KeyboardEvent& evt)
	{
		using namespace OgreBites;

	    if (evt.keysym.sym == SDLK_ESCAPE)
	    {
	        getRoot()->queueEndRendering();
	    }
	    else if(evt.keysym.sym == SDLK_F3)
		{
	        mDebugOn = !mDebugOn;

			if (!mDebugOn)
				mDbgDraw->clear();
		}
		else if (evt.keysym.sym == SDLK_LSHIFT)
		{
			//this->neck->pitch(Ogre::Radian(0.1));	
			//hand->removeChild("mNinjaNode");

			this->bananoR = mDynWorld->addRigidBody(5, mNinjaEntity, BtOgre::CT_SPHERE);
			this->bananoR->setActivationState(1);

			double direccion[3] = {0.0, 0.0, 0.0};
			double potencia = 5;

			_cprintf("\nIngrese el vector de dirección (separado por espacios - Doubles): \n");
			_cscanf("%lf %lf %lf", &direccion[0], &direccion[1], &direccion[2]);
			_cprintf("\nIngrese la potencia del disparo (Double): \n");
			_cscanf("%lf", &potencia);			
			this->bananoR->applyCentralImpulse(btVector3(direccion[0] * potencia, direccion[1] * potencia, direccion[2] * potencia));			

			if (turno) {
				Ogro2ContactResultCallback callbackOgro2(this);
				mDynWorld->getBtWorld()->contactPairTest(bananoR, ogro2R, callbackOgro2);
			}
			else {
				OgroContactResultCallback callbackOgro(this);
				mDynWorld->getBtWorld()->contactPairTest(bananoR, ogroR, callbackOgro);
			}

			MapaContactResultCallback callbackMapa(this);
			mDynWorld->getBtWorld()->contactPairTest(bananoR, mapaR, callbackMapa);

		}
		else if (evt.keysym.sym == SDLK_UP)
		{
			if (this->turno) {
				this->mOgro2Node->translate(Ogre::Vector3(0, 0, 1));
				ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);
			}
			else {
				this->mOgroNode->translate(Ogre::Vector3(0, 0, -1));
				ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
			}
			agarrarPelota();
		}
		else if (evt.keysym.sym == SDLK_DOWN)
		{
			if (this->turno) {
				this->mOgro2Node->translate(Ogre::Vector3(0, 0, -1));
				ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);
			}
			else {
				this->mOgroNode->translate(Ogre::Vector3(0, 0, 1));
				ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
			}
			agarrarPelota();
		}
		else if (evt.keysym.sym == SDLK_RIGHT)
		{
			if (this->turno) {
				this->mOgro2Node->translate(Ogre::Vector3(-1, 0, 0));
				ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);
			}
			else {
				this->mOgroNode->translate(Ogre::Vector3(1, 0, 0));
				ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
			}
			agarrarPelota();
		}
		else if (evt.keysym.sym == SDLK_LEFT)
		{
			if (this->turno) {
				this->mOgro2Node->translate(Ogre::Vector3(1, 0, 0));
				ogro2R = mDynWorld->addRigidBody(5, mOgro2Entity, BtOgre::CT_BOX);
			}
			else {
				this->mOgroNode->translate(Ogre::Vector3(-1, 0, 0));
				ogroR = mDynWorld->addRigidBody(5, mOgroEntity, BtOgre::CT_BOX);
			}
			agarrarPelota();
		}
		else if (evt.keysym.sym == SDLK_SPACE)
		{
			cambiarTurno();
		}
	    return true;
	}

	void cambiarTurno() {
		this->turno = !this->turno;	
		if (mDynWorld->getBtWorld()->getCollisionObjectArray().size() > 3) {
			//mDynWorld->getBtWorld()->removeCollisionObject(mDynWorld->getBtWorld()->getCollisionObjectArray()[3]);
		}
		agarrarPelota();
	}

	void agarrarPelota() {
		if (this->turno) {
			Ogre::SkeletonInstance* skl = mOgro2Entity->getSkeleton();
			hand = skl->getBone("Hand.R");
			/*hand->setManuallyControlled(true);*/
			//hand->addChild(mNinjaNode);
			//_cprintf("MANO POSICION: %lf, %lf, %lf", hand->_getDerivedPosition()[0] + mOgro2Node->getPosition()[0], hand->_getDerivedPosition()[1] + mOgro2Node->getPosition()[1], hand->_getDerivedPosition()[2] + +mOgro2Node->getPosition()[2]);
			//mNinjaNode->setPosition(hand->_getDerivedPosition() + mOgro2Node->getPosition());		
			mNinjaNode->setPosition(mOgro2Node->getPosition());
			//mNinjaNode->setPosition(hand->getPosition());
			mNinjaNode->translate(Ogre::Vector3(0, 0, 2));
		}
		else {
			Ogre::SkeletonInstance* skl = mOgroEntity->getSkeleton();
			hand = skl->getBone("Hand.R");
			/*hand->setManuallyControlled(true);*/
			//hand->addChild(mNinjaNode);
			//_cprintf("MANO POSICION: %lf, %lf, %lf", hand->_getDerivedPosition()[0] + mOgroNode->getPosition()[0], hand->_getDerivedPosition()[1] + mOgroNode->getPosition()[1], hand->_getDerivedPosition()[2] + +mOgroNode->getPosition()[2]);
			//mNinjaNode->setPosition(hand->_getDerivedPosition() + mOgroNode->getPosition());
			//mNinjaNode->setPosition(obtenerPosManoR(mOgroNode->getPosition()));
			mNinjaNode->setPosition(mOgroNode->getPosition());
			//mNinjaNode->setPosition(hand->getPosition());
			mNinjaNode->translate(Ogre::Vector3(0, 0, -2));
		}		
		
	}

	Ogre::Vector3 GetBoneWorldPosition(Entity* ent, Bone bone)
	{
		Vector3 world_position = bone._getDerivedPosition();

		//multiply with the parent derived transformation
		Node* pParentNode = ent->getParentNode();
		SceneNode* pSceneNode = ent->getParentSceneNode();
		while (pParentNode != NULL)
		{
			//process the current i_Node
			if (pParentNode != pSceneNode)
			{
				//this is a tag point (a connection point between 2 entities). which means it has a parent i_Node to be processed
				/*Ogre::TagPoint* tp = static_cast<Ogre::TagPoint*> (pParentNode);
				world_position = tp->_getFullLocalTransform() * world_position;
				pParentNode = (pParentNode as TagPoint).ParentEntity.ParentNode;*/
			}
			else
			{
				//this is the scene i_Node meaning this is the last i_Node to process
				world_position = pParentNode->_getFullTransform() * world_position;
				break;
			}
		}
		return world_position;
	}

	Ogre::Vector3 obtenerPosManoR(Ogre::Vector3 pos) {
		Ogre::Vector3 suma = Ogre::Vector3(0, 0, 0);
		Ogre::Node* nodo = hand;
		while (nodo->getParent() != NULL) {
			suma = suma + nodo->getPosition();
			nodo = nodo->getParent();
		}
		suma[0] += pos[0];
		suma[1] -= 5;
		suma[2] += pos[2];
		//_cprintf("SUMA: %lf, %lf, %lf", suma[0], suma[1], suma[2]);
		return suma;
	}

    bool frameStarted(const FrameEvent &evt)
    {
        OgreBites::ApplicationContext::frameStarted(evt);

        //Update Bullet world. Don't forget the debugDrawWorld() part!
        mDynWorld->getBtWorld()->stepSimulation(evt.timeSinceLastFrame, 10);

        if(mDebugOn)
        	mDbgDraw->update();

        return true;
    }
};

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  main() function. Need say more?
 * =====================================================================================
 */

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
#else
int main(int argc, char **argv)
#endif
{
    // Create application object
	//showWin32Console();
	AllocConsole();	

    BtOgreTestApplication app;
    app.initApp();
    app.getRoot()->startRendering();
    app.closeApp();	

	FreeConsole();
	std::cout << "Hola2";

    return 0;
}
