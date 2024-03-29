#ifndef RENDER_ENGINE_RENDER_VKVF_H_
#define RENDER_ENGINE_RENDER_VKVF_H_

#include <functional>
#include <span>
#include <array>
#include <variant>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <windows.h>

#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

#include "../src/stl_util.h"

#pragma warning(push, 0)
#include "tiny_gltf.h"
#pragma warning(pop)
namespace render
{
#ifdef WIN32
	using InitParam = HINSTANCE;
#endif

	template<typename ElementType>
	static std::span<const ElementType> GetBufferSpanByAccessor(const tinygltf::Model& gltf_model, int acc_ind)
	{
		auto&& buffer_acc = gltf_model.accessors[acc_ind];
		auto&& buffer_view = gltf_model.bufferViews[buffer_acc.bufferView];

		auto overriden_stride = buffer_view.byteStride;
		auto element_size = tinygltf::GetNumComponentsInType(buffer_acc.type) * tinygltf::GetComponentSizeInBytes(buffer_acc.componentType);

		assert(sizeof(ElementType) == element_size);

		auto actual_stride = overriden_stride > 0 ? overriden_stride : element_size;

		auto offset = buffer_view.byteOffset + buffer_acc.byteOffset;


		const ElementType* begin = reinterpret_cast<const ElementType*>(gltf_model.buffers[buffer_view.buffer].data.data() + offset);
		const ElementType* end = begin + (buffer_acc.count);

		std::span<const ElementType> result(begin, end);

		return result;
	}

	enum class ObjectType
	{
#define RENDER_ENGINE_OBJECTS
#include "render_engine_objects.inl"
	};



	template<ObjectType Type>
	struct ObjectId
	{
		ObjectId() : value(-1) {}
		ObjectId(const std::string& name, uint32_t id) : name(name), value(id) {}
		std::string name;
		uint32_t value;
	};

	struct InputState
	{
		std::array<int, 0xFF> button_states;
		std::pair<int, int> mouse_position;
	};


	template<ObjectType Type>
	struct ObjectDescription
	{
		std::string name;
	};

	template<>
	struct ObjectDescription<ObjectType::StaticModel>
	{
		std::string pack_name;
		std::string model_name;

		std::string name;
	};

	template<>
	struct ObjectDescription<ObjectType::Bitmap>
	{
		std::string image_path;

		std::string name;
	};


	template<>
	struct ObjectDescription<ObjectType::DbgPoints>
	{
		std::vector<glm::vec3> points;
		glm::vec4 color;

		std::string name;
	};




	namespace command
	{
		struct Load
		{
			std::string pack_name;
			std::shared_ptr<tinygltf::Model> model;
		};

		struct Image
		{
			std::string name;
			std::string path;
		};

		struct SetActiveCameraNode
		{
			ObjectId<ObjectType::Node> node_id;
		};

		struct DbgPoints
		{
			std::vector<glm::vec3> poists;
			glm::vec4 color;
		};

		struct Geometry
		{
			std::vector<glm::vec3> faces;
		};

		struct ObjectsUpdate
		{
			std::vector<std::pair<uint32_t, glm::mat4>> updates;
		};

		template<typename T>
		struct CallMethod;

		template<typename Ret, class ObjType, typename ... Params, typename ... Params2>
		CallMethod(Ret(ObjType::*)(Params...), Params2...) -> CallMethod<Ret(ObjType::*)(Params...)>;

		template<typename Ret, class ObjType, typename ... Params>
		struct CallMethod<Ret(ObjType::*)(Params...)>
		{

			using MehodPtr = Ret(ObjType::*)(Params...);

			std::function<void(ObjType&)> f_;
			uint32_t object_id_;

			CallMethod(Ret (ObjType::* method)(Params...), Params ... args) : f_([=](ObjType& obj) { (obj.*method)(args...);  })
			{

			}
		};


		template<ObjectType Type>
		struct AddObject
		{
			uint32_t object_id;
			ObjectDescription<Type> desc;
		};

#define RENDER_ENGINE_OBJECT(x) AddObject<ObjectType::x>,
		using Command = std::variant<
#include "render_engine_objects.inl"
			Load, Image, Geometry, ObjectsUpdate, SetActiveCameraNode
		>;
	}
	

	class IRenderEngineInstance
	{
	public:

	};

	class RenderEngine
	{
	public:
		RenderEngine(InitParam param, const std::string& app_name);

		//void CreateGraphicsPipeline();
		void StartRender();

		const InputState& GetInputState();

		struct DebugPoint
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		void SetDebugLines(const std::vector<std::pair<DebugPoint, DebugPoint>>& lines);

		template<ObjectType Type>
		ObjectId<Type> AddObject(const ObjectDescription<Type>& desc);

		//void UpdateCamera(uint32_t id, glm::vec3 pos, glm::vec3 dir);

		void QueueCommand(const command::Command& render_command);

		~RenderEngine();

		bool VKInitSuccess();
		//class RenderEngineImpl;
	private:
		class RenderEngineImpl;
		std::unique_ptr<RenderEngineImpl> impl_;
	};

	namespace ui
	{
		struct TextBlockProxy
		{
			void SetText(const std::string& text)
			{
				int a = 1;
			}


		//	TextBlockProxy(render::RenderEngine::RenderEngineImpl& reimpl);
		//public:

		};
	}
}
#endif  // RENDER_ENGINE_RENDER_VKVF_H_