library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_lut is
   port (   
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_lut_waddr      : out std_logic_vector(7 downto 0);
      o_lut_wdata      : out std_logic_vector(7 downto 0);
      o_lut_wren       : out std_logic
   );
end entity;

-- command to fill the color lut with values.
-- expected byte sequence: 0x10,<lut0>,<lut1>,...,<lut255> - fill lut arbitrarily
-- expected byte sequence: 0x11 - fill lut linearly
-- expected byte sequence: 0x12 - fill lut logarithmicly
architecture rtl of cmd_lut is   

   type T_STATE is ( CMD, WRITE_LUT, FILL_LIN, FILL_LOG );  
   signal s_state     : T_STATE;
   signal s_cmd       : integer;
   signal s_idx       : integer;
   signal s_wren       : std_logic;

begin
   process (i_reset_n, i_clk)
   variable v_writenext : std_logic;
   begin
      if i_reset_n = '0' then
         s_state <= CMD;
         o_busy <= '0';
         o_need_more_data <= '0';         
         s_wren <= '0';
            
      elsif rising_edge(i_clk) then
                       
          if s_state = CMD then
            if i_data_rdy = '1' then
               o_busy <= '1';
               s_idx <= 0;
               case i_data is 
               when X"10" =>  -- write lut
                  o_need_more_data <= '1';
                  s_state <= WRITE_LUT;
               when X"11" =>  -- fill lut linear
                  s_state <= FILL_LIN;
               when X"12" =>  -- fill lut logarithmic
                  s_state <= FILL_LOG;
               when others =>
                  o_busy <= '0';
               end case;
            end if;
         elsif s_wren = '1' then
            s_wren <= '0';
         else
            o_lut_waddr <= std_logic_vector(to_unsigned(s_idx, o_lut_waddr'length));
            v_writenext := '0';
            
            case s_state is             
            when WRITE_LUT =>
               if i_data_rdy = '1' then
                  o_lut_wdata <= i_data;
                  v_writenext := '1';
               end if;             
            when FILL_LIN =>
               o_lut_wdata <= std_logic_vector(to_unsigned(s_idx, o_lut_wdata'length));
               v_writenext := '1';
            when FILL_LOG =>         
               s_state <= CMD;
            when others =>           
            end case;
            
            if v_writenext = '1' then
               s_wren <= '1';
               s_idx <= s_idx + 1;
               if s_idx = 255 then
                  s_state <= CMD;
                  o_busy <= '0';
                  o_need_more_data <= '0';
               end if;
            end if;            
         end if;
         o_lut_wren <= s_wren;
      end if;      
     
   end process;   
end architecture;
            
            
            
            









   -- type tLUT is array (0 to 255) of std_logic_vector(7 downto 0);
   -- variable v_lut : tLUT := ( -- default LUT with linear-logarithmic intensity mapping
      -- X"00", X"00", X"00", X"00", X"01", X"01", X"01", X"01",
      -- X"02", X"02", X"02", X"02", X"03", X"03", X"03", X"04",
      -- X"04", X"04", X"05", X"05", X"05", X"05", X"06", X"06",
      -- X"06", X"07", X"07", X"07", X"08", X"08", X"08", X"09",
      -- X"09", X"09", X"0A", X"0A", X"0A", X"0B", X"0B", X"0B",
      -- X"0C", X"0C", X"0D", X"0D", X"0D", X"0E", X"0E", X"0E",
      -- X"0F", X"0F", X"10", X"10", X"10", X"11", X"11", X"12",
      -- X"12", X"13", X"13", X"13", X"14", X"14", X"15", X"15",
      -- X"16", X"16", X"17", X"17", X"18", X"18", X"18", X"19",
      -- X"19", X"1A", X"1A", X"1B", X"1B", X"1C", X"1C", X"1D",
      -- X"1E", X"1E", X"1F", X"1F", X"20", X"20", X"21", X"21",
      -- X"22", X"22", X"23", X"24", X"24", X"25", X"25", X"26",
      -- X"27", X"27", X"28", X"28", X"29", X"2A", X"2A", X"2B",
      -- X"2C", X"2C", X"2D", X"2E", X"2E", X"2F", X"30", X"30",
      -- X"31", X"32", X"32", X"33", X"34", X"35", X"35", X"36",
      -- X"37", X"38", X"38", X"39", X"3A", X"3B", X"3C", X"3C",
      -- X"3D", X"3E", X"3F", X"40", X"40", X"41", X"42", X"43",
      -- X"44", X"45", X"46", X"47", X"47", X"48", X"49", X"4A",
      -- X"4B", X"4C", X"4D", X"4E", X"4F", X"50", X"51", X"52",
      -- X"53", X"54", X"55", X"56", X"57", X"58", X"59", X"5A",
      -- X"5B", X"5C", X"5E", X"5F", X"60", X"61", X"62", X"63",
      -- X"64", X"65", X"67", X"68", X"69", X"6A", X"6C", X"6D",
      -- X"6E", X"6F", X"71", X"72", X"73", X"74", X"76", X"77",
      -- X"78", X"7A", X"7B", X"7C", X"7E", X"7F", X"81", X"82",
      -- X"84", X"85", X"87", X"88", X"89", X"8B", X"8D", X"8E",
      -- X"90", X"91", X"93", X"94", X"96", X"98", X"99", X"9B",
      -- X"9D", X"9E", X"A0", X"A2", X"A3", X"A5", X"A7", X"A9",
      -- X"AA", X"AC", X"AE", X"B0", X"B2", X"B4", X"B5", X"B7",
      -- X"B9", X"BB", X"BD", X"BF", X"C1", X"C3", X"C5", X"C7",
      -- X"C9", X"CB", X"CE", X"D0", X"D2", X"D4", X"D6", X"D8",
      -- X"DB", X"DD", X"DF", X"E1", X"E4", X"E6", X"E8", X"EB",
      -- X"ED", X"F0", X"F2", X"F4", X"F7", X"F9", X"FC", X"FF"
   -- );
