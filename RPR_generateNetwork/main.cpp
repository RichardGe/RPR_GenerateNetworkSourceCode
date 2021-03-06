
#include "tinyxml2.h"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>

struct RPR_FUNCTION_ARGUMENT
{
	std::string argName;
	std::string type;
	bool is_out;
};

struct RPR_FUNCTION
{
	std::string fnName;
	std::string returnType;
	std::vector<RPR_FUNCTION_ARGUMENT> args;
};




void ExtractFunctionFromXML(std::vector<RPR_FUNCTION>& rprFnList , std::map<std::string,std::string>& typedefFromXML)
{
	std::cout<<"ExtractFunctionFromXML..." << std::endl;

	tinyxml2::XMLDocument doc;
	doc.LoadFile( "RadeonProRender.src.xml"); 
	tinyxml2::XMLNode* rootnode = doc.FirstChild();
	tinyxml2::XMLNode* SiblingNode = rootnode->NextSibling();
	tinyxml2::XMLElement* material_element = SiblingNode->ToElement();

	std::string material_element_name = material_element->Name(); // "material"
	
	tinyxml2::XMLNode* ChildNode = SiblingNode ->FirstChild();
	tinyxml2::XMLElement* ChildElmt = ChildNode->ToElement();
	const char* strTagName = ChildElmt->Name(); // "comment"

	for(int i=0; ;i++)
	{
		tinyxml2::XMLNode* SiblingNode = ChildNode->NextSibling();

		if ( SiblingNode == nullptr )
		{
			break;
		}

		tinyxml2::XMLElement* material_element = SiblingNode->ToElement();

		std::string elem_name = material_element->Name();

		if ( elem_name == "function" )
		{
			RPR_FUNCTION newFunction;

			newFunction.fnName = material_element->Attribute("name");
			newFunction.returnType = material_element->Attribute("return_type");
			
			//if ( material_element_attib_name == "rprContextCreateHeteroVolume" )
			{
				tinyxml2::XMLNode* ChildNode7 = SiblingNode ->FirstChild();

				for(int k=0; ;k++)
				{
					RPR_FUNCTION_ARGUMENT newArg;

					tinyxml2::XMLElement* ChildElmt201 = ChildNode7->ToElement();
					const char* strTagName = ChildElmt201->Name(); //"arg"
					std::string  arg1name = ChildElmt201->Attribute("name");
					int aa=0;

					newArg.argName = arg1name;
					newArg.type = ChildElmt201->Attribute("type");
					newArg.is_out = false;

					const tinyxml2::XMLAttribute* attrib_isOut = ChildElmt201->FindAttribute("is_out");
					if ( attrib_isOut )
					{
						std::string is_out = ChildElmt201->Attribute("is_out");
						if ( is_out == "true" )
						{
							newArg.is_out = true;
						}
					}
					

					newFunction.args.push_back(newArg);

					tinyxml2::XMLNode* SiblingNode101 = ChildNode7->NextSibling();
					if ( SiblingNode101 == 0 )
					{
						break;
					}

					ChildNode7 = SiblingNode101;
				}
			}
			rprFnList.push_back(newFunction);
		}
		else if ( elem_name == "typedef" )
		{
			std::string name = material_element->Attribute("name");
			std::string type = material_element->Attribute("type");

			typedefFromXML[name] = type;
		}

		ChildNode = SiblingNode;
	}

	return;
}

const char headerMessageComment[] = "// Do not Edit this file Manually\r\n// this file is auto-generated by RPR_GenerateNetworkSourceCode\r\n// https://github.com/RichardGe/RPR_GenerateNetworkSourceCode\r\n";

void ExportToOpcodeH(const std::vector<RPR_FUNCTION>& rprFnList, const std::map<std::string,std::string>& typedefFromXML)
{
	std::cout<<"ExportToOpcodeH..." << std::endl;


	std::ofstream networkH;
	networkH.open ("rprApiNetworkOpcode.h" , std::ofstream::binary  |  std::ofstream::trunc );

	if ( !networkH.is_open() || networkH.fail() )
	{
		std::cout<<"ERROR open out file" << std::endl;
		return;
	}

	for(int i=0; i<30;i++)
		networkH <<"// \r\n";
	networkH <<headerMessageComment;
	for(int i=0; i<30;i++)
		networkH <<"// \r\n";



	//networkH << "#pragma once\r\n";

	for(int iFn=0; iFn<rprFnList.size(); iFn++)
	{
		networkH << "#define RPRNET_OPCODE_";
		networkH << rprFnList[iFn].fnName;
		networkH << " ";
		networkH << std::to_string(1 + iFn);
		networkH << "\r\n";
	}

	networkH.close();

	return;
}

void ExportToNetworkFunctionCaller(const std::vector<RPR_FUNCTION>& rprFnList, const std::map<std::string,std::string>& typedefFromXML)
{
	std::cout<<"ExportToNetworkFunctionCaller..." << std::endl;

	std::ofstream networkH;
	networkH.open ("RprApiNetworkFunctionCaller.h" , std::ofstream::binary  |  std::ofstream::trunc );

	if ( !networkH.is_open() || networkH.fail() )
	{
		std::cout<<"ERROR open out file" << std::endl;
		return;
	}

	for(int i=0; i<30;i++)
		networkH <<"// \r\n";
	networkH <<headerMessageComment;
	for(int i=0; i<30;i++)
		networkH <<"// \r\n";


	networkH << "#pragma once\r\n\r\n\r\n";


	for(int iFn=0; iFn<rprFnList.size(); iFn++)
	{
		networkH << "#ifdef RPR_USE_RPRNET\r\n";
		networkH << "#define NETWORK_CALL_";
		networkH << rprFnList[iFn].fnName ;
		networkH << "(node___) \\\r\n";
		networkH << "{\\\r\n";
		networkH << "if ( node___ )\\\r\n";
		networkH << "{\\\r\n";
		networkH << "FrNode* ctx__ = (node___->GetType() == NodeTypes::Context) ? (node___) : (node___->GetProperty<FrNode*>(FR_NODE_CONTEXT));\\\r\n";
		networkH << "if ( ctx__ )\\\r\n";
		networkH << "{\\\r\n";
		networkH << "std::shared_ptr<RprApiNetwork> networkMgr = ctx__->GetProperty<  std::shared_ptr<RprApiNetwork>  >(FR_NETWORK_MANAGER);\\\r\n";
		networkH << "if ( networkMgr ) { networkMgr->api_";
		networkH << rprFnList[iFn].fnName ;
		networkH << "(";
		for(int iArg=0; iArg<rprFnList[iFn].args.size(); iArg++)
		{
			networkH << rprFnList[iFn].args[iArg].argName;
			if ( iArg != rprFnList[iFn].args.size()-1 )
			{
				networkH << " , ";
			}
		}
		networkH << "); }\\\r\n";

		networkH << "}\\\r\n";
		networkH << "}\\\r\n";
		networkH << "}\r\n";
		networkH << "#else\r\n";
		networkH << "#define NETWORK_CALL_";
		networkH <<  rprFnList[iFn].fnName;
		networkH << "(node___)\r\n";
		networkH << "#endif\r\n\r\n";

	}

	networkH << "/// END OF FILE\r\n";

	networkH.close();

	return;

}

void ExportToNetworkH(const std::vector<RPR_FUNCTION>& rprFnList, const std::map<std::string,std::string>& typedefFromXML)
{
	

	std::cout<<"ExportToNetworkH..." << std::endl;



	std::ofstream networkH;
	networkH.open ("RprApiNetwork.h" , std::ofstream::binary  |  std::ofstream::trunc );

	if ( !networkH.is_open() || networkH.fail() )
	{
		std::cout<<"ERROR open out file" << std::endl;
		return;
	}

	for(int i=0; i<30;i++)
		networkH <<"// \r\n";
	networkH <<headerMessageComment;
	for(int i=0; i<30;i++)
		networkH <<"// \r\n";


	networkH << "#pragma once\r\n";
	networkH << "#ifdef RPR_USE_RPRNET\r\n";
	networkH << "#include \"RprNetwork.h\"\r\n";
	networkH << "class RprApiNetwork : public RprNetwork\r\n{\r\npublic:\r\n";
	

	for(int iFn=0; iFn<rprFnList.size(); iFn++)
	{
		networkH << "\t";
		networkH << rprFnList[iFn].returnType;
		networkH << " api_";
		networkH << rprFnList[iFn].fnName ;
		networkH << "(";

		for(int iArg=0; iArg<rprFnList[iFn].args.size(); iArg++)
		{
			networkH << rprFnList[iFn].args[iArg].type;
			networkH << " ";
			networkH << rprFnList[iFn].args[iArg].argName;
			if ( iArg != rprFnList[iFn].args.size()-1 )
			{
				networkH << " , ";
			}
		}

		networkH << ");\r\n";

	}

	networkH << "\r\n};\r\n";

	networkH << "#endif // ifdef RPR_USE_RPRNET\r\n";
	networkH.close();

	return;
}

void ExportToNetworkCpp(const std::vector<RPR_FUNCTION>& rprFnList, const std::map<std::string,std::string>& typedefFromXML)
{

	std::cout<<"ExportToNetworkCpp..." << std::endl;

	std::ofstream networkCpp;
	networkCpp.open ("RprApiNetwork.cpp" , std::ofstream::binary  |  std::ofstream::trunc );

	if ( !networkCpp.is_open() || networkCpp.fail() )
	{
		std::cout<<"ERROR open out file" << std::endl;
		return;
	}

	for(int i=0; i<30;i++)
		networkCpp <<"// \r\n";
	networkCpp <<headerMessageComment;
	for(int i=0; i<30;i++)
		networkCpp <<"// \r\n";

	networkCpp << "const int g_heteroVolume_componentCountPerLookup = 3; // this value shouldn't be modified\r\n";

	networkCpp << "#ifdef RPR_USE_RPRNET\r\n";
	networkCpp << "#include \"RprApiNetwork.h\"\r\n";
	//networkCpp << "#include \"RprApiNetworkOpcode.h\"\r\n";
	for(int iFn=0; iFn<rprFnList.size(); iFn++)
	{
		networkCpp << rprFnList[iFn].returnType;
		networkCpp << " RprApiNetwork::api_";
		networkCpp << rprFnList[iFn].fnName ;
		networkCpp << "(";

		for(int iArg=0; iArg<rprFnList[iFn].args.size(); iArg++)
		{
			networkCpp << rprFnList[iFn].args[iArg].type;
			networkCpp << " ";
			networkCpp << rprFnList[iFn].args[iArg].argName;
			if ( iArg != rprFnList[iFn].args.size()-1 )
			{
				networkCpp << " , ";
			}
		}

		networkCpp << ")\r\n{\r\n";

		
		

		if (   rprFnList[iFn].fnName == "rprShapeGetInfo"
			|| rprFnList[iFn].fnName == "rprMeshGetInfo"
			|| rprFnList[iFn].fnName == "rprCurveGetInfo"
			|| rprFnList[iFn].fnName == "rprHeteroVolumeGetInfo"
			|| rprFnList[iFn].fnName == "rprGridGetInfo"
			|| rprFnList[iFn].fnName == "rprBufferGetInfo"
			|| rprFnList[iFn].fnName == "rprMeshPolygonGetInfo"
			|| rprFnList[iFn].fnName == "rprLightGetInfo"
			|| rprFnList[iFn].fnName == "rprSceneGetInfo"
			|| rprFnList[iFn].fnName == "rprFrameBufferGetInfo"
			|| rprFnList[iFn].fnName == "rprMaterialNodeGetInfo"
			|| rprFnList[iFn].fnName == "rprCompositeGetInfo"
			|| rprFnList[iFn].fnName == "rprPostEffectGetInfo"
			|| rprFnList[iFn].fnName == "rprContextGetInfo"
			|| rprFnList[iFn].fnName == "rprCameraGetInfo"
			|| rprFnList[iFn].fnName == "rprImageGetInfo"
			|| rprFnList[iFn].fnName == "rprInstanceGetBaseShape"
			|| rprFnList[iFn].fnName == "rprMaterialSystemGetSize"
			|| rprFnList[iFn].fnName == "rprMaterialNodeGetInputInfo"
			|| rprFnList[iFn].fnName == "rprContextGetAttachedPostEffectCount"
			|| rprFnList[iFn].fnName == "rprContextGetParameterInfo"
			|| rprFnList[iFn].fnName == "rprObjectSetName"

			|| rprFnList[iFn].fnName == "rprContextCreateCurveBatch"   // TODO : manage it ( lower priority )
			|| rprFnList[iFn].fnName == "rprContextCreateCurve"        // TODO : manage it ( lower priority )
			|| rprFnList[iFn].fnName == "rprContextCreateHeteroVolume" // TODO : manage it ( lower priority )
			)
		{
			networkCpp << "\t// ignore this function for network\r\n" ;
		}
		else
		{


			networkCpp << "\t//if a config is missing, return directly\r\n";
			//networkCpp << "\tif (  m_address == \"\" || m_port == \"\" || m_sessionid == \"\" || m_renderid == \"\" )\r\n" ;
			networkCpp << "\tif (  !NetworkValuesReady() )\r\n" ;
			networkCpp << "\t\treturn RPR_SUCCESS;\r\n";
			networkCpp << "\tm_bufferReceiver.cursor = 0;\r\n";
			networkCpp << "\t\r\n";
			//networkCpp << "\t#pragma pack(push,1)\r\n";
			//networkCpp << "\tstruct api_arguments\r\n";
			//networkCpp << "\t{\r\n";

			//networkCpp << "\t\tunsigned char opCode;\r\n";

			std::string fillStruct = "";

			for(int iArg=0; iArg<rprFnList[iFn].args.size() ;iArg++)
			{
				std::string argName = "arg" + std::to_string(iArg);

				//networkCpp << "\t\t";

				std::string type = rprFnList[iFn].args[iArg].type;


				if ( typedefFromXML.find(type) != typedefFromXML.end() )
				{
				 	std::string correspondingType = typedefFromXML.at(type);
					if ( correspondingType == "rpr_uint" )
					{
						type = correspondingType;
					}
					else if ( correspondingType == "rpr_bitfield" )
					{
						type = "rpr_uint";
					}
				}

				





				bool isOut = rprFnList[iFn].args[iArg].is_out;

				if ( 
					!isOut
					&&
					(   type == "rpr_uint" 
					||  type == "rpr_float"  
					||  type == "rpr_int"  
					||  type == "size_t" 
					||  type == "rpr_bool" 
					)
					)
				{
					//std types

					//networkCpp << type ;

					//fillStruct += "\targInput->" + argName + " = ";
					//fillStruct += rprFnList[iFn].args[iArg].argName;
					//fillStruct += ";\r\n";


					fillStruct += "\tchain::Arg* blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += " = blockNewCall->add_args(); blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += "->set_value((const void*)&";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += ",sizeof(";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += "));\r\n";



				}
				else if ( 
					!isOut
					&&
					(   type == "rpr_context" 
					||  type == "rpr_scene"  
					||  type == "rpr_light" 
					||  type == "rpr_shape" 
					||  type == "rpr_buffer" 
					||  type == "rpr_image" 
					||  type == "rpr_hetero_volume"
					||  type == "rpr_camera"
					||  type == "rpr_curve"
					||  type == "rpr_material_node"
					||  type == "rpr_framebuffer"
					||  type == "rpr_material_system"
					||  type == "rpr_composite"
					||  type == "rpr_lut"
					||  type == "rpr_post_effect"
					||  type == "rpr_grid"
					|| type == "void *" && rprFnList[iFn].fnName == "rprObjectDelete" // special case : rprObjectDelete is void*
					)
					)
				{
					// id representing RPR object

					//networkCpp << "void*";

					//fillStruct += "\targInput->" + argName + " = (void*)";
					//fillStruct += rprFnList[iFn].args[iArg].argName;
					//fillStruct += ";\r\n";


					fillStruct += "\tchain::Arg* blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += " = blockNewCall->add_args(); blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += "->set_value((const void*)&";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += ",sizeof(";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += "));\r\n";

				
				}
				else if ( 
					isOut
					&&
					(   type == "rpr_context *" 
					||  type == "rpr_scene *"  
					||  type == "rpr_light *" 
					||  type == "rpr_shape *" 
					||  type == "rpr_buffer *" 
					||  type == "rpr_image *" 
					||  type == "rpr_hetero_volume *"
					||  type == "rpr_camera *"
					||  type == "rpr_curve *"
					||  type == "rpr_material_node *"
					||  type == "rpr_framebuffer *"
					||  type == "rpr_material_system *"
					||  type == "rpr_composite *"
					||  type == "rpr_lut *"
					||  type == "rpr_post_effect *"
					||  type == "rpr_grid *"
					)
					)
				{
					// id representing RPR object

					//networkCpp << "void*";

					//fillStruct += "\targInput->" + argName + " = (void*)(*";
					//fillStruct += rprFnList[iFn].args[iArg].argName;
					//fillStruct += ");\r\n";


					fillStruct += "\tchain::Arg* blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += " = blockNewCall->add_args(); blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += "->set_value((const void*)";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += ",sizeof(*";
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += "));\r\n";


				}

				else if (
					!isOut
					&&
					(
					    type == "rpr_float const **"
					||  type == "rpr_int const **"
					)
					)
				{
					//networkCpp << "uint64_t";


					if (  ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" ||  rprFnList[iFn].fnName == "rprContextCreateMeshEx2")
						&& rprFnList[iFn].args[iArg].argName == "texcoords" )
					{
						//fillStruct += "\tPushNewBinData_texcoords(argInput->";
						//fillStruct += argName;
						//fillStruct += ", texcoords ,   numberOfTexCoordLayers , num_texcoords  ,texcoord_stride );\r\n";


						fillStruct += "\tchain::Arg* blockArg_";
						fillStruct += std::to_string(iArg);
						fillStruct += " = blockNewCall->add_args();";
						fillStruct += "\tPushNewBinData_texcoords2(";
						fillStruct += "blockArg_";
						fillStruct += std::to_string(iArg);
						fillStruct += ", texcoords ,   numberOfTexCoordLayers , num_texcoords  ,texcoord_stride );\r\n";
					}
					else if (  ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" ||  rprFnList[iFn].fnName == "rprContextCreateMeshEx2")
						&& rprFnList[iFn].args[iArg].argName == "texcoord_indices" )
					{
						//fillStruct += "\tPushNewBinData_texcoord_indices(argInput->";
						//fillStruct += argName;
						//fillStruct += ", texcoord_indices  ,   numberOfTexCoordLayers ,  num_faces,   num_face_vertices,  tidx_stride );\r\n";

						fillStruct += "\tchain::Arg* blockArg_";
						fillStruct += std::to_string(iArg);
						fillStruct += " = blockNewCall->add_args();";
						fillStruct += "\tPushNewBinData_texcoord_indices2(";
						fillStruct += "blockArg_";
						fillStruct += std::to_string(iArg);
						fillStruct += ", texcoord_indices  ,   numberOfTexCoordLayers ,  num_faces,   num_face_vertices,  tidx_stride );\r\n";
					}
					else
					{
						//networkCpp << "????????????????";
						fillStruct += "????????????????";
					}

					
					

					int aa=0;

				}

				else if (
				
					!isOut
					&&
					(  
						type == "rpr_char const *"  || type == "const rpr_char *" 
					||  type == "rpr_int const *" 
					||  type == "rpr_float const *" 
					||  type == "rpr_uint const *"
					||  type == "rpr_context_properties const *"
					||  type == "rpr_image_format const"
					||  type == "rpr_framebuffer_format const"
					||  type == "rpr_image_desc const *"
					||  type == "void const *"
					||  type == "rpr_buffer_desc const *"
					||  type == "size_t const *"
					||  type == "rpr_mesh_info const *"
					||  type == "rpr_framebuffer_desc const *"
					)
					)
				{
					// export on network as buffer
					//networkCpp << "uint64_t";

					
					std::string sizeDataStr = "SIZE_TODO";
					if ( type == "rpr_char const *" || type == "const rpr_char *"  )
					{
						sizeDataStr = rprFnList[iFn].args[iArg].argName;
						sizeDataStr += " ? ( strlen(";
						sizeDataStr += rprFnList[iFn].args[iArg].argName;
						sizeDataStr += ")+1) : 0";
					}
					else if ( rprFnList[iFn].fnName == "rprCreateContext" && rprFnList[iFn].args[iArg].argName == "pluginIDs" )
						sizeDataStr = "0";
					else if ( rprFnList[iFn].fnName == "rprCreateContext" && rprFnList[iFn].args[iArg].argName == "props" )
						sizeDataStr = "0";

					else if ( rprFnList[iFn].fnName == "rprContextCreateImage" && rprFnList[iFn].args[iArg].argName == "format" )
						sizeDataStr = "sizeof(rpr_image_format)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateImage" && rprFnList[iFn].args[iArg].argName == "image_desc" )
						sizeDataStr = "sizeof(rpr_image_desc)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateImage" && rprFnList[iFn].args[iArg].argName == "data" )
						sizeDataStr = "ComputeDataSize_rprContextCreateImage_data(format,image_desc)";


					else if ( rprFnList[iFn].fnName == "rprContextCreateBuffer" && rprFnList[iFn].args[iArg].argName == "buffer_desc" )
						sizeDataStr = "sizeof(rpr_buffer_desc)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateBuffer" && rprFnList[iFn].args[iArg].argName == "data" )
						sizeDataStr = "ComputeDataSize_rprContextCreateBuffer_data(buffer_desc)";
					
					
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "vertices" )
						sizeDataStr = "num_vertices*vertex_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "normals" )
						sizeDataStr = "num_normals*normal_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "texcoords" )
						sizeDataStr = "num_texcoords*texcoord_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "vertex_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMesh_vertex_indices(num_face_vertices,  num_faces ,  vidx_stride)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "normal_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMesh_normal_indices( num_face_vertices,  num_faces ,  vidx_stride,  nidx_stride)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "texcoord_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMesh_texcoord_indices(num_face_vertices,  num_faces ,  vidx_stride ,  tidx_stride)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMesh" && rprFnList[iFn].args[iArg].argName == "num_face_vertices" )
						sizeDataStr = "num_faces * sizeof(num_face_vertices[0])";

					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "vertices" )
						sizeDataStr = "num_vertices*vertex_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "normals" )
						sizeDataStr = "num_normals*normal_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "perVertexFlag" )
						sizeDataStr = "num_perVertexFlags*perVertexFlag_stride";
					//else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "texcoords" )
					//	sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_texcoords()";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "num_texcoords" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(size_t)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "texcoord_stride" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(rpr_int)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "vertex_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_vertex_indices(num_faces ,  num_face_vertices ,  vidx_stride)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "normal_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_normal_indices(num_faces ,   num_face_vertices ,    nidx_stride)";
					//else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "texcoord_indices" )
					//	sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_texcoord_indices()";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "tidx_stride" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(rpr_int)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx" && rprFnList[iFn].args[iArg].argName == "num_face_vertices" )
						sizeDataStr = "num_faces * sizeof(num_face_vertices[0])";


					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "vertices" )
						sizeDataStr = "num_vertices*vertex_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "normals" )
						sizeDataStr = "num_normals*normal_stride";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "perVertexFlag" )
						sizeDataStr = "num_perVertexFlags*perVertexFlag_stride";
					//else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "texcoords" )
					//	sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx2_texcoords()";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "num_texcoords" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(size_t)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "texcoord_stride" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(rpr_int)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "vertex_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_vertex_indices(num_faces ,  num_face_vertices ,  vidx_stride)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "normal_indices" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx_normal_indices(num_faces ,   num_face_vertices ,    nidx_stride)";
					//else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "texcoord_indices" )
					//	sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx2_texcoord_indices()";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "tidx_stride" )
						sizeDataStr = "numberOfTexCoordLayers*sizeof(rpr_int)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "num_face_vertices" )
						sizeDataStr = "num_faces * sizeof(num_face_vertices[0])";
					else if ( rprFnList[iFn].fnName == "rprContextCreateMeshEx2" && rprFnList[iFn].args[iArg].argName == "mesh_properties" )
						sizeDataStr = "ComputeDataSize_rprContextCreateMeshEx2_mesh_properties(mesh_properties)";


					else if ( rprFnList[iFn].fnName == "rprContextCreateFrameBuffer" && rprFnList[iFn].args[iArg].argName == "format" )
						sizeDataStr = "sizeof(rpr_framebuffer_format)";
					else if ( rprFnList[iFn].fnName == "rprContextCreateFrameBuffer" && rprFnList[iFn].args[iArg].argName == "fb_desc" )
						sizeDataStr = "sizeof(rpr_framebuffer_desc)";


					else if ( rprFnList[iFn].fnName == "rprCameraSetTransform" && rprFnList[iFn].args[iArg].argName == "transform" )
						sizeDataStr = "sizeof(float)*16";

					else if ( rprFnList[iFn].fnName == "rprCurveSetTransform" && rprFnList[iFn].args[iArg].argName == "transform" )
						sizeDataStr = "sizeof(float)*16";

					else if ( rprFnList[iFn].fnName == "rprShapeSetTransform" && rprFnList[iFn].args[iArg].argName == "transform" )
						sizeDataStr = "sizeof(float)*16";

					else if ( rprFnList[iFn].fnName == "rprLightSetTransform" && rprFnList[iFn].args[iArg].argName == "transform" )
						sizeDataStr = "sizeof(float)*16";

					else if ( rprFnList[iFn].fnName == "rprHeteroVolumeSetTransform" && rprFnList[iFn].args[iArg].argName == "transform" )
						sizeDataStr = "sizeof(float)*16";


					else if ( rprFnList[iFn].fnName == "rprShapeSetMaterialFaces" && rprFnList[iFn].args[iArg].argName == "face_indices" )
						sizeDataStr = "num_faces*sizeof(rpr_int)";

					else if ( rprFnList[iFn].fnName == "rprContextCreateGrid" && rprFnList[iFn].args[iArg].argName == "gridData" )
						sizeDataStr = "gridDataSizeByte";

					else if ( rprFnList[iFn].fnName == "rprHeteroVolumeSetEmissionLookup" && rprFnList[iFn].args[iArg].argName == "ptr" )
						sizeDataStr = "g_heteroVolume_componentCountPerLookup*sizeof(float)*n";

					else if ( rprFnList[iFn].fnName == "rprHeteroVolumeSetDensityLookup" && rprFnList[iFn].args[iArg].argName == "ptr" )
						sizeDataStr = "g_heteroVolume_componentCountPerLookup*sizeof(float)*n";

					else if ( rprFnList[iFn].fnName == "rprHeteroVolumeSetAlbedoLookup" && rprFnList[iFn].args[iArg].argName == "ptr" )
						sizeDataStr = "g_heteroVolume_componentCountPerLookup*sizeof(float)*n";

					else if ( rprFnList[iFn].fnName == "rprContextCreateGrid" && rprFnList[iFn].args[iArg].argName == "indicesList" )
						sizeDataStr = "ComputeDataSize_rprContextCreateGrid_indicesList(indicesListTopology,numberOfIndices)";

					else
					{
						sizeDataStr = "SIZE_TODO_";
					}
					

					//fillStruct += "\tPushNewBinData(argInput->";
					//fillStruct += argName;
					//fillStruct += ",(void*)";
					//if ( type == "rpr_image_format const" || type == "rpr_framebuffer_format const" )
					//{
					//	fillStruct += "&";
					//}
					//fillStruct += rprFnList[iFn].args[iArg].argName;
					//fillStruct += ",";
					//fillStruct += sizeDataStr;
					//fillStruct += ");\r\n";




					fillStruct += "\tchain::Arg* blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += " = blockNewCall->add_args();";
					fillStruct += "PushNewBinData2(blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += ",(void*)";
					if ( type == "rpr_image_format const" || type == "rpr_framebuffer_format const" )
					{
						fillStruct += "&";
					}
					fillStruct += rprFnList[iFn].args[iArg].argName;
					fillStruct += ",";
					fillStruct += sizeDataStr;
					fillStruct += ");\r\n";

					int aa=0;
				}
				else
				{
					//networkCpp << "????";

					//fillStruct += "\targInput->" + argName + " = ";
					//fillStruct += "????";
					//fillStruct += " //";
					//fillStruct += rprFnList[iFn].args[iArg].argName;
					//fillStruct += "\r\n";

					fillStruct += "\tchain::Arg* blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += " = blockNewCall->add_args(); blockArg_";
					fillStruct += std::to_string(iArg);
					fillStruct += "->set_value(??????);\r\n";

					int a=0;
				}

				//networkCpp << " ";
				//networkCpp << argName;
				//networkCpp << ";\r\n";


			}
			//networkCpp << "\t};\r\n";
			//networkCpp << "\t#pragma pack(pop)\r\n";
			//networkCpp << "\t\r\n";
			//networkCpp << "\tconst int sizeofStruct = sizeof(api_arguments);\r\n";

			networkCpp << "\t\r\n";
			networkCpp << "\tchain::RprCall* blockNewCall = m_currentBlock.add_calls();\r\n";
			networkCpp << "\tblockNewCall->set_func_name(\""+rprFnList[iFn].fnName+"\");\r\n\r\n";

			//networkCpp << "\tapi_arguments* argInput = (api_arguments*)PrepareCommand(sizeof(api_arguments));\r\n";
			//networkCpp << "\targInput->opCode = RPRNET_OPCODE_"+rprFnList[iFn].fnName+";\r\n";
			networkCpp << fillStruct;
			//networkCpp << "\tm_bufferToSend.sizeleft = sizeofStruct;\r\n";

			networkCpp << "\t\r\n";





		}


		

		networkCpp << "\tEndApiFunction(\"";
		networkCpp << rprFnList[iFn].fnName;
		networkCpp << "\");\r\n";
		networkCpp << "\treturn RPR_SUCCESS;\r\n";
		networkCpp << "}\r\n";

	}

	networkCpp << "#endif // ifdef RPR_USE_RPRNET\r\n";

	networkCpp.close();


	return;
}

int main()
{
	std::cout<<"Program Start" << std::endl;

	std::vector<RPR_FUNCTION> rprFnList;

	// example :  ["rpr_environment_override"] = "rpr_uint"
	std::map<std::string,std::string> typedefFromXML;
	
	ExtractFunctionFromXML(rprFnList,typedefFromXML);
	
	//ExportToOpcodeH(rprFnList,typedefFromXML);
	ExportToNetworkH(rprFnList,typedefFromXML);
	ExportToNetworkCpp(rprFnList,typedefFromXML);
	ExportToNetworkFunctionCaller(rprFnList,typedefFromXML);

	std::cout<<"Program End" << std::endl;

    return 0;
}




