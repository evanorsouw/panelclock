library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
	port ( 	
		clk       : in std_logic;
		reset     : in std_logic;
		data      : in std_logic_vector (5 downto 0);
		--
		addr      : out std_logic_vector (14 downto 0);
		dsp_clk   : out std_logic;
		dsp_latch : out std_logic;
		dsp_oe    : out std_logic;
		dsp_addr  : out std_logic_vector (4 downto 0);
		dsp_r1    : out std_logic;
		dsp_g1    : out std_logic;
		dsp_b1    : out std_logic;
		dsp_r2    : out std_logic;
		dsp_g2    : out std_logic;
		dsp_b2    : out std_logic
	);
end entity FM6124;

architecture FM6124_arch of FM6124 is

	
begin
	Clock_Proc: process (clk, reset)
	variable clk_count    : unsigned (1 downto 0);
	variable column_count : unsigned (8 downto 0);
	variable row_count    : unsigned (4 downto 0);
	variable pixclk       : std_logic; 
	variable oe           : std_logic;
	variable latch        : std_logic;
	variable pixel_count  : unsigned (14 downto 0);
	variable waiting      : std_logic;
	variable depth_count  : unsigned (7 downto 0);
	variable depth_delay  : unsigned (7 downto 0);
	begin
		if (reset = '1') then
			clk_count    := "00";
			column_count := "000000000";
			row_count    := "00000";
			pixclk       := '0';
			latch        := '0';
			oe           := '1';
			waiting      := '0';
			depth_count  := "00000000";
			depth_delay  := "10000000";
			
		elsif rising_edge(clk) then
			
			clk_count := clk_count + 1;
			
			if (clk_count = 0) then
				pixclk := '0';
				
			elsif (clk_count = 1) then			
				dsp_r1 <= std_logic_vector(data)(0);
				dsp_g1 <= std_logic_vector(data)(1);
				dsp_b1 <= std_logic_vector(data)(2);
				dsp_r2 <= std_logic_vector(data)(3);
				dsp_g2 <= std_logic_vector(data)(4);
				dsp_b2 <= std_logic_vector(data)(5);
				
			elsif (clk_count = 2) then
			   if (waiting = '1') then
					depth_count := depth_count + 1;
					if (depth_count = depth_delay) then
						waiting := '0';
						if (row_count = 31) then
							depth_delay := depth_delay(0) & depth_delay(7 downto 1);
							if (depth_delay = "10000000") then 
								pixel_count := "000000000000000";
							end if;
						end if;
					end if;
				else
					if (column_count < 128) then 
						pixclk := '1';
						pixel_count := pixel_count + 1;
					elsif (column_count = 128) then
						oe := '1';
					elsif (column_count = 129) then
						latch  := '1';
					elsif (column_count = 130) then
						latch := '0';
						row_count := row_count + 1;
					elsif (column_count = 131) then
						oe := '0';
						waiting := '1';
						depth_count := "00000000";
					end if;
					if (waiting = '1') then
						column_count := "000000000";
					else
						column_count := column_count + 1;
					end if;
				end if;
				
			end if;			
		end if;
		
		addr      <= std_logic_vector(pixel_count);
		dsp_clk   <= pixclk;
		dsp_oe    <= oe;
		dsp_latch <= latch;
		dsp_addr  <= std_logic_vector(row_count);
		
	end process;
	
	
end architecture FM6124_arch;
				
				
				
				