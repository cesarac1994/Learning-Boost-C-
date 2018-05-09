/***Protocolo de comunicação
*Envio de mensagem: origem|destino|texto_ 
*Conexão: origem||_
*Desconexão: origem|_|_
*/

#include "stdafx.h" //função de cabeçalho visual studio 2017
#include <iostream>
#include <boost/array.hpp>
#include <cmath>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\thread\mutex.hpp>

using boost::asio::ip::tcp;
const char caracterSeparador = '|';
const char caracterFimMsg = '_';
boost::mutex chave;
void destrinche_msg(std::string, std::string&, std::string&, std::string&);
void empacotar_msg(std::string&, std::string, std::string, std::string);
void ler_buffer(boost::asio::ip::tcp::socket&);
void escreve_buffer(boost::asio::ip::tcp::socket&, std::string);
bool criar_conexao(boost::asio::ip::tcp::socket&, std::string);
void mandar_msg(boost::asio::ip::tcp::socket&, std::string);
void menu_principal(boost::asio::ip::tcp::socket&, std::string);
void desconectar(boost::asio::ip::tcp::socket&, std::string);

int main(int argc, char* argv[])
{
	std::string nickname;
	std::cout << "CHAT" << std::endl;
	std::cout << "Digite seu nick: ";
	std::cin.getline(nickname);
	try
	{
		boost::asio::io_service io_service;
		tcp::resolver resolver(io_service);
		tcp::resolver::query query("127.0.0.1", "13");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);	
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	if(criar_conexao(socket,nickname)){	
		boost::thread t2(boost::bind(menu_principal, socket, nickname);
		t2.join();
	}
	else{
		std::cout << "Conexão negada pelo servidor, \nMotivo: seu nickname já estar sendo utilizado por outro usuário!\n";
	}
	system("PAUSE");
	return 0;
}

//Criar conexão
bool criar_conexao(boost::asio::ip::tcp::socket& socket, std::string nickname) {
	std::string pacote_estabelecer_conexao, resposta_servidor, nao_aceitou_conexao;
	empacotar_msg(pacote_estabelecer_conexao, nickname, "", "");
	escreve_buffer(socket, pacote_estabelecer_conexao);
	ler_buffer(socket,resposta_servidor);
	empacotar_msg(nao_aceitou_conexao, caracterFimMsg, caracterFimMsg);
	if(resposta_servidor != nao_aceitou_conexao){ //se já tiver nick no servidor
		return true;
	}  
	return false;
}

void destrinche_msg(std::string msg, std::string& orig, std::string& dest, std::string& corpo) {
	std::vector<std::string> tudo_separado;
	boost::split(tudo_separado, msg, [](char c) {return c == caracterSeparador;});
	orig = tudo_separado[0];
	dest = tudo_separado[1];
	corpo = tudo_separado[2];
}

void empacotar_msg(std::string& msg, std::string orig, std::string dest, std::string corpo) {
	msg = orig + caracterSeparador + dest + caracterSeparador + corpo + caracterFimMsg;
}

void ler_buffer(boost::asio::ip::tcp::socket& scket, std::string& mensagem) {
	for (;;)
	{
		boost::array<char, 128> buf;
		boost::system::error_code error;
		size_t len = scket.read_some(boost::asio::buffer(buf), error);
		if (error == boost::asio::error::eof)
			break; // Connection closed cleanly by peer.
		else if (error)
			throw boost::system::system_error(error); // Some other error.
		mensagem += buf.data();
	}
}

void escreve_buffer(boost::asio::ip::tcp::socket& scket, std::string msg) {
	try{
		boost::system::error_code error;
		chave.lock();
		size_t len = scket.write_some(boost::asio::buffer(msg,msg.length()), error);
		chave.unlock();
		if (error)
			throw boost::system::system_error(error); // Some other error.
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl; //salvar no arquivo tambem
	}
}

void desconectar(boost::asio::ip::tcp::socket& socket, std::string nickname){
	std::string mensagem;
	empacotar_msg(mensagem, nickname, caracterFimMsg, "");
	escreve_buffer(socket, mensagem);
}

void mandar_msg(boost::asio::ip::tcp::socket& socket, std::string nickname){
	std::string dest,texto,msg;
	std::cout << "Digite o destino ";
	cin.getline(dest);
	std::cout << "Digite a mensagem ";
	cin.getline(texto);
	empacotar_msg(msg, nickname, dest, texto);
	escreve_buffer(socket, msg);
}



void menu_principal(boost::asio::ip::tcp::socket& socket, std::string nickname){	
	enum menu_opcoes{
		op_mandar_msg=1,
		op_ver_usuarios=2,
		op_desconectar=3	
	};
	int opcao;
	switch(opcao){
		std::cout << "1 - Mandar mensagem\n 2 - Exibir quem está online\n 3 - Desconectar\n";
		std::cout << "Digite a opção: ";
		std::cin >> opcao;
		chave.unlock();			
		case op_mandar_msg:
			mandar_msg(socket,nickname);
			break;
		case op_ver_usuarios:
			break;
		case op_desconectar:
			desconectar(socket, nickname);
			break;
		default:
			std::cout << "Opção inválida";
	}
}

