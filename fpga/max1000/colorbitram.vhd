library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity color_bitram is
	port ( 	
		rd_clock  : in std_logic;
		rd_addr   : in std_logic_vector (13 downto 0);
		wr_clock  : in std_logic;
		wr_addr   : in std_logic_vector (10 downto 0);
		wr_data   : in std_logic_vector (7 downto 0);
      wr_en     : in std_logic;
		--
		q         : out std_logic
	);
end entity color_bitram;

architecture color_bitram_arch of color_bitram is	
	
	component bitram
	PORT
	(
		rdclock		: IN STD_LOGIC;
		data		   : IN STD_LOGIC_VECTOR (7 DOWNTO 0);
		rdaddress	: IN STD_LOGIC_VECTOR (10 DOWNTO 0);
      wrclock     : in std_logic;
		wraddress	: IN STD_LOGIC_VECTOR (10 DOWNTO 0);
		wren	      : IN STD_LOGIC;
		--
		q		      : OUT STD_LOGIC_VECTOR (7 DOWNTO 0)
	);
	end component;	

	component byte_demux
	port ( 	
		byte      : in std_logic_vector (7 downto 0);
		addr      : in std_logic_vector (2 downto 0);
		--
		q         : out std_logic
	);
	end component;
	
	signal color_byte   : std_logic_vector (7 downto 0);

begin

	RAM : bitram
	port map (
		rdclock     => rd_clock,
		data		   => wr_data,
		rdaddress	=> rd_addr (10 downto 0),
		wrclock     => wr_clock,
		wraddress	=> wr_addr,
		wren	      => wr_en,
		q		      => color_byte
	);
	
	demux : byte_demux
	port map (
		byte        => color_byte,
		addr        => rd_addr(13 downto 11),
		q           => q
	);
	
end architecture color_bitram_arch;