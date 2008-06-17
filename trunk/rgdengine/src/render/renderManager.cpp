#include "precompiled.h"

#include <rgde/render/renderManager.h>
#include <rgde/render/renderBinders.h>
#include <rgde/render/renderLightManager.h>
#include <rgde/render/renderDevice.h>
#include <rgde/render/renderCameraManager.h>

#include <rgde/base/lexical_cast.h>

#include <rgde/core/application.h>

#include <d3dx9.h>
#include "renderTextureImpl.h"


extern LPDIRECT3DDEVICE9 g_pd3dDevice;

namespace render
{
	PTexture safeLoadDefaultTexture(const std::string &strTextureName)
	{
		io::ScopePathAdd p	("Common/");
		PTexture pResult	= ITexture::Create(strTextureName);

		if (!pResult)
		{
			base::lerr << "Can't load default texture \"" << strTextureName << "\"";
			core::IApplication::Get()->close();
		}

		return pResult;
	}

	RenderManager::RenderManager()
		: 	m_bLightingEnabled(true),
			m_nFillMode(Solid),
			m_bVolumes(true),
			m_pWhiteTexture(safeLoadDefaultTexture("White.jpg")),
			m_pDefaultNormalMap(safeLoadDefaultTexture("DefaultNormalMap.jpg")),
			m_pBlackTexture(safeLoadDefaultTexture("Black.jpg")),
			m_pDefaultEffect(Effect::Create("Default.fxo")),
			m_pDefaultFont(IFont::Create(11,  L"Arial", render::IFont::Heavy))			
	{

		m_pDefaultFog.loadFromXML("Default.xml");
		m_pCurrentFog = m_pDefaultFog;


		if (!m_pDefaultEffect)
		{
			base::lerr << "Can't load effect \"graphics/shaders/Default.fxo\"";
			core::IApplication::Get()->close();
		}

		if (!m_pDefaultFont)
		{
			base::lerr << "Can't create font \"" << "Arial" << "\"";
			core::IApplication::Get()->close();
		}
	}

	RenderManager::~RenderManager()
	{
	}

	PEffect& RenderManager::getDefaultEffect()
	{
		return m_pDefaultEffect;
	}

	PFont& RenderManager::getDefaultFont()
	{
		return m_pDefaultFont;
	}

	PTexture& RenderManager::getBlackTexture()
	{
		return m_pBlackTexture;
	}

	PTexture& RenderManager::getWhiteTexture()
	{
		return m_pWhiteTexture;
	}

	PTexture& RenderManager::getDefaultNormalMap()
	{
		return m_pDefaultNormalMap;
	}

	void RenderManager::setCurrentFog(const Fog &pFog)
	{
		m_pCurrentFog = pFog;
	}

	void RenderManager::add(IRendererable *r)
	{
		m_lRenderables.push_back(r);
	}

	void RenderManager::remove(IRendererable *r)
	{
		//m_lRenderables.remove(r);
		Renderables::iterator it = std::find(m_lRenderables.begin(), m_lRenderables.end(), r);
		m_lRenderables.erase(it);
	}

	void RenderManager::clear()
	{
		m_lRenderables.resize(0);
	}

	namespace functors
	{
		//void setupParameters(PEffect pEffect, const SRenderableInfo &info, PMaterial& mat)
		//{
		//	//assert(info.pFrame);
		//	if (info.pFrame)
		//	{
		//		m_woldMatrixBinder.setFrame(*(info.pFrame));
		//		m_woldMatrixBinder.setupParameters();		
		//		mat->bind();
		//	}
		//	
		//}

		struct SDefaultRender
		{
			PEffect& defaultEffect;
			SDefaultRender() 
				: defaultEffect(TheRenderManager::Get().getDefaultEffect())
			{
			}

			void operator()(SRenderableInfo const * r)
			{
				if (NULL == r) return;
				render(*r);
			}

			inline void render(const SRenderableInfo &info)
			{
				if(info.pFrame)
				{
					static PMaterial pDefaultMaterial = Material::Create();

					//const PMaterial& pMaterial = info.spMaterial ? info.spMaterial : pDefaultMaterial;
					//const PEffect&	 pEffect	= info.spShader ? info.spShader : defaultEffect;

					const PMaterial& pMaterial = pDefaultMaterial;
					const PEffect&	 pEffect	= defaultEffect;


					//m_pDefaultEffect
					
					pMaterial->getDynamicBinder()->setupParameters(info.pFrame);
									
					Effect::ITechnique *pTechnique = pMaterial->getTechnique();

					if(NULL != pTechnique)
					{	
						pTechnique->begin();
						//base::lmsg << "effect tech -=" << pTechnique->getName() << "=- begin";

						std::vector<Effect::ITechnique::IPass*>   &vecPasses = pTechnique->getPasses();

						for (size_t i = 0; i < vecPasses.size(); i++)
						{
							Effect::ITechnique::IPass	*pass = vecPasses[i];
							pass->begin();
								info.pRenderFunc();
							pass->end();
						}

						pTechnique->end();
						//base::lmsg << "effect tech <" << pTechnique->getName() << "> end";
					}
					else
					{
						//info.pFrame->getFullTransform()
						//return mProj*(mView*frame->getFullTransform());
						g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&info.pFrame->getFullTransform()[0]);
						//g_pd3dDevice->Set
						info.pRenderFunc();
						//base::lmsg << "Invalid binder or technique";
					}
				}
				else
				{
					info.pRenderFunc();
				}
			}

			inline void renderDebug(SRenderableInfo &info)
			{
				if (info.pDebugRenderFunc != NULL)
					info.pDebugRenderFunc();
			}
		};

		struct SPrioritySorter_Less
		{
			bool operator()(IRendererable *r1, IRendererable *r2)
			{
				return r1->getPriority() < r2->getPriority() ? true : false;
			}
		};

		struct SDistanceSorter_Less
		{
			math::Vec3f	vCamPos;
			SDistanceSorter_Less(const math::Vec3f &camPos)
				: vCamPos(camPos)
			{
			}

			bool operator()(SRenderableInfo const * r1, SRenderableInfo const * r2)
			{
				const math::PFrame& pFrame1 = r1->pFrame;
				const math::PFrame& pFrame2 = r2->pFrame;

				math::Vec3f		pos1, pos2;
				if (pFrame1)
					pos1 = pFrame1->getGlobalPosition();
				if (pFrame2)
					pos2 = pFrame2->getGlobalPosition();
				float	fLengthSqr1			= math::lengthSquared<float, 3>(vCamPos - pos1),
						fLengthSqr2			= math::lengthSquared<float, 3>(vCamPos - pos2);

				return fLengthSqr1 > fLengthSqr2;
			}
		};
	}

	struct SRenderblesSorter
	{
		std::vector<SRenderableInfo const *>   &vsolids;
		std::vector<SRenderableInfo const *>   &vtrans;
		std::vector<SRenderableInfo const *>   &vposttrans;
		const math::Frustum				   &m_frustum;

		SRenderblesSorter(std::vector<SRenderableInfo const *> &solids, std::vector<SRenderableInfo const *> &trans, std::vector<SRenderableInfo const *> &posttrans, const math::Frustum &frustum)
			: vsolids(solids),
			  vtrans(trans),
			  vposttrans(posttrans),
			  m_frustum(frustum)
		{
		}

		void operator()(IRendererable const * r)
		{
			if ((NULL == r) || (r->isVisible() == false))
				return;

			const SRenderableInfo  &ri = r->getRenderableInfo();

			if (ri.pFrame)
			{
				const math::Point3f& max	= ri.bbox.getMax();
				const math::Point3f& min	= ri.bbox.getMin();
				math::Point3f center		= min + (max - min) / 2.0f;

				float fHalfLenght = math::length<float, 3>(max - min) / 2.0f;

				math::Point3f centerGlobal = ri.pFrame->getFullTransform() * center;

				if (!m_frustum.CubeInFrustum(centerGlobal[0], centerGlobal[1], centerGlobal[2], fHalfLenght))
					return;
			}

			if (r->getPriority() >= 1000)
				vposttrans.push_back(&ri);
			else if (ri.spMaterial && ri.spMaterial->isTransparent())
				vtrans.push_back(&ri);
			else
				vsolids.push_back(&ri);
		}
	};	

	void RenderManager::renderScene()
	{
		render::TheDevice::Get().resetStats();

		//m_lRenderables.sort(functors::SPrioritySorter_Less());
		std::sort(m_lRenderables.begin(), m_lRenderables.end(), functors::SPrioritySorter_Less());

		TheCameraManager::Get().sortCameras();

		static std::vector<SRenderableInfo const *> vPostTransparet(1000);
		static std::vector<SRenderableInfo const *> vTransparet(1000);
		static std::vector<SRenderableInfo const *> vSolid(1000);

		// draw scene through every active camera
		CameraManager &cm	= TheCameraManager::Get();
		if (cm.begin() != cm.end())
		{
			for (CameraManager::CameraListIterator camera = cm.begin(); camera != cm.end(); ++camera)
			{
				vSolid.resize(0);
				vTransparet.resize(0);
				vPostTransparet.resize(0);

				TheCameraManager::Get().setCamera(camera);

				if(!m_pStaticBinder)
					createBinder();
				m_pStaticBinder->setupParameters(0);

				const math::Frustum& frustum = TheDevice::Get().getCurentCamera()->getFrustum();
				std::for_each(m_lRenderables.begin(), m_lRenderables.end(), SRenderblesSorter(vSolid, vTransparet, vPostTransparet, frustum));

				int nVisibleObjects = static_cast<int>(vTransparet.size() + vSolid.size());
				//std::string str = base::Lexical_cast<std::string, int>(nVisibleObjects);
				//std::wstring wstr(str.begin(), str.end());
				//getDefaultFont()->renderText(wstr, math::Rect(1, 29, 400, 400), 0xFFFFFFFF, true);

				std::sort(vTransparet.begin(), vTransparet.end(), functors::SDistanceSorter_Less(TheDevice::Get().getCurentCamera()->getPosition()));

				{
					{
						const math::PCamera& pCamera = *camera;
						const math::Matrix44f& mView = pCamera->getViewMatrix();
						const math::Matrix44f& mProj = pCamera->getProjMatrix();
						//return mProj*(mView*frame->getFullTransform());
						g_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&mView[0]);
						g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&mProj[0]);
					}


					functors::SDefaultRender r;
					std::for_each(vSolid.begin(),			vSolid.end(),			r);
					std::for_each(vTransparet.begin(),		vTransparet.end(),		r);
					std::for_each(vPostTransparet.begin(),	vPostTransparet.end(),	r);
				}
			}
		}
		else // �� ������ ���� ��� ���� �������� ���-�� � �������� ����������� � ������
		{
			if(!m_pStaticBinder) 
				createBinder();
			m_pStaticBinder->setupParameters(0);

			functors::SDefaultRender r;

			for (Renderables::iterator it = m_lRenderables.begin();
				it != m_lRenderables.end(); ++it)
			{
				if ((*it)->isVisible())
					r(&(*it)->getRenderableInfo());
			}
		}

		// draw debug information
		//scene::TheScene::Get().debugDraw();
		//render::TheDevice::Get().showStatistics(getDefaultFont());
	}

	void RenderManager::createBinder()
	{
		m_pStaticBinder = createStaticBinder(m_pDefaultEffect);
	}

	IRendererable::IRendererable(unsigned priority)
		: m_nRenderPriority(priority),
		  m_bIsVisible(true)
	{
		TheRenderManager::Get().add(this);
	}

	IRendererable::~IRendererable()
	{
		if (TheRenderManager::IsCreated())
			TheRenderManager::Get().remove(this);
	}


	SRenderableInfo::SRenderableInfo()
		: pFrame(0),
		  bHaveVolumes(false),
		  spMaterial(Material::Create())
	{
	}
}