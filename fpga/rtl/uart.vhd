library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity UART is
   port (   
      i_clkx         : in std_logic;   -- clock at least 8x the required bitrate (e.g. 8*9600)
      i_idle_ticks   : integer;        -- typically the number of ticks to sent 2 bytes at lowest baudrate
      i_reset_n      : in std_logic;
      i_rx           : in std_logic;   -- the nput uart signal
      --
      o_datain       : out std_logic_vector (7 downto 0);
      o_datain_clk   : out std_logic

      -- uncomment next statements for debugging in testbench
      -- t_state           : out integer;
      -- t_duration        : out signed(31 downto 0);
      -- t_iread           : out integer;
      -- t_iwrite          : out integer;
      -- t_count           : out integer;
      -- t_sumdurations    : out signed(31 downto 0);
      -- t_minduration     : out signed(31 downto 0);
      -- t_active_duration : out signed(31 downto 0);
      -- t_bitcount        : out integer;
      -- t_assembled_byte  : out std_logic_vector(7 downto 0)
   );
end entity;

-- serial byte
-- ______      ____ ____ ____ ____ ____ ____ ____ ____ ____._______
--       \____X____X____X____X____X____X____X____X____X     
--  idle  Start  D0  D1   D2   D3   D4   D5   D6   D7  stop   idle
-- 
-- ______           ____.____      ____.____.____.____.____._____--________
--       \____.____/         \____/                     
--         Sta  D0   D1   D2   D3   D4   D5   D6   D7   D8   Stp
--       |         |         |    |                                |
--       start     10        10   5                                X (timeout)
--
-- Time measuring always starts on falling edge, so first duration represents a '0'.
-- A timeout at the end of the last byte must always include the stopbit so that represents a '1'
-- The duration table for above example (byte 0xF6): [10,10,5,X]
-- smallest is 5
-- convert duration table into bits:
-- (10/5=2) '0' '0' (10/5=2) '1' '1' (5/5=1) '0' (0=timeout fill up remaining 5 bits) '1' '1' '1' '1' '1' 
-- resulting bitstream:
--    S01234567P
--    0011011111 => byte 0xF6
-- 
architecture UART_arch of UART is   

   constant MINIMUM_TRANSITIONS : integer := 6;
   constant FIFOSIZE            : integer := MINIMUM_TRANSITIONS + 1;

   type tState is ( WAIT_IDLE, WAIT_START, MEASURE, PROCESS_STORED, PROCESS_LIVE );
   type tArray is array (0 to FIFOSIZE-1) of signed(31 downto 0);
   
   signal s_state           : tState;
   signal s_last_rx         : std_logic;
   signal s_duration        : signed(31 downto 0);
   signal s_minduration     : signed(31 downto 0);
   signal s_iwrite          : integer range 0 to FIFOSIZE-1;
   signal s_iread           : integer range 0 to FIFOSIZE-1;
   signal s_count           : integer range 0 to FIFOSIZE;
   signal s_durations       : tArray;
   
   signal s_active_duration : signed(31 downto 0);
   signal s_assembled_byte  : std_logic_vector(7 downto 0);
   signal s_next_bit_value  : std_logic; 
   signal s_bitcount        : integer range 0 to 10;
   signal s_datain          : std_logic_vector(7 downto 0);
   signal s_datain_clk      : std_logic;

begin
   process (i_clkx, i_reset_n)
   variable v_active : boolean;
   begin     
      if i_reset_n = '0' then
         s_state <= WAIT_IDLE;
         s_duration <= to_signed(0,s_duration'length);
         s_active_duration <= to_signed(0,s_active_duration'length);
         s_bitcount <= 0;
            
      elsif rising_edge(i_clkx) then       
         
         s_datain_clk <= '0';
         s_last_rx <= i_rx;

         case s_state is
         when WAIT_IDLE =>
            -- wait for an idle period on the line to start measuring cycle
            if i_rx = '0' then
               s_duration <= to_signed(0,s_duration'length);
               s_iwrite <= 0;
               s_iread <= 0;               
               s_count <= 0;
            else
               s_duration <= s_duration + 1;
               if s_duration = i_idle_ticks then
                  s_state <= WAIT_START;
               end if;
            end if;
         when WAIT_START =>
            -- wait for a transition to 0 representing the startbit
            if i_rx = '0' then
               s_duration <= to_signed(1,s_duration'length);
               s_minduration <= to_signed(i_idle_ticks,s_minduration'length);
               s_last_rx <= i_rx;
               s_state <= MEASURE;
            end if;
         when MEASURE =>  
            -- store transition times and measure bit duration
            s_duration <= s_duration + 1;
            s_last_rx <= i_rx;
            if i_rx /= s_last_rx then
               if s_count < FIFOSIZE then
                  if s_duration >= i_idle_ticks then
                     s_durations(s_iwrite) <= to_signed(i_idle_ticks,s_duration'length);
                  else            
                     s_durations(s_iwrite) <= s_duration;
                     if s_duration < s_minduration then
                        s_minduration <= s_duration;
                     end if;
                  end if;
                  s_iwrite <= (s_iwrite + 1) mod FIFOSIZE;
                  s_count <= s_count + 1;                             
                  if s_count > MINIMUM_TRANSITIONS - 1 then
                     s_bitcount <= 0;
                     s_state <= PROCESS_STORED;
                  end if;
               else
                  s_state <= WAIT_IDLE;
                  s_iread <= 0;
                  s_iwrite <= 0;
                  s_count <= 0;
               end if;
               s_duration <= to_signed(1,s_duration'length);
            end if;
         when PROCESS_STORED =>                     
            -- keep storing transition times while in this state.
            s_duration <= s_duration + 1;
            if i_rx /= s_last_rx then
               s_durations(s_iwrite) <= s_duration;
               s_iwrite <= (s_iwrite + 1) mod FIFOSIZE;
               s_count <= s_count + 1;
               s_duration <= to_signed(1,s_duration'length);
            else
               case s_bitcount is
               when 0 =>  -- startbit, must always start from a new transition
                  if s_count = 0 then
                     s_state <= PROCESS_LIVE;  -- only exit to live on start of new byte
                     s_bitcount <= 1;
                  else
                     s_active_duration <= s_durations(s_iread) - s_minduration;
                     s_iread <= (s_iread + 1) mod FIFOSIZE;
                     s_count <= s_count - 1;
                     s_bitcount <= 2;
                     s_next_bit_value <= '0';
                  end if;
               when 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 =>
                  if s_active_duration = i_idle_ticks then
                     -- last duration is timeout so fill remainder with '1';
                     s_assembled_byte <= '1' & s_assembled_byte(7 downto 1);
                     s_bitcount <= s_bitcount + 1;
                  elsif s_active_duration >= (s_minduration / 2) then
                     -- can still extract a bit from last duration
                     s_active_duration <= s_active_duration - s_minduration;
                     s_assembled_byte <= s_next_bit_value & s_assembled_byte(7 downto 1);
                     s_bitcount <= s_bitcount + 1;
                  elsif s_count > 0 then
                     -- byte incomplete but new duration already available.
                     if s_durations(s_iread) = i_idle_ticks then
                        -- duration is timeout this means '1's by definition
                        s_active_duration <= s_durations(s_iread);
                        s_assembled_byte <= '1' & s_assembled_byte(7 downto 1);
                     else
                        s_active_duration <= s_durations(s_iread) - s_minduration;
                        s_next_bit_value <= not s_next_bit_value;
                        s_assembled_byte <= (not s_next_bit_value) & s_assembled_byte(7 downto 1);
                     end if;
                     s_iread <= (s_iread + 1) mod FIFOSIZE;
                     s_count <= s_count - 1;                     
                     s_bitcount <= s_bitcount + 1;
                  end if;
               when 10 =>
                  -- stopbit
                  if s_active_duration > s_minduration / 2 then
                     s_datain <= s_assembled_byte;                  
                     s_datain_clk <= '1';
                     s_bitcount <= 0;
                  elsif s_count > 0 then
                     s_iread <= (s_iread + 1) mod FIFOSIZE;
                     s_count <= s_count - 1;                     
                     s_datain <= s_assembled_byte;                  
                     s_datain_clk <= '1';
                     s_bitcount <= 0;
                  end if;
               when others =>
               end case;
            end if;
         when PROCESS_LIVE =>
            s_duration <= s_duration + 1;
            case s_bitcount is
            when 0 => -- startbit
               if i_rx = '0' then                  
                  s_bitcount <= 1;
                  s_duration <= to_signed(0, s_duration'length);
               end if;
            when 1 => 
               if s_duration >= s_minduration / 2 then
                  if i_rx = '1' or s_duration = s_minduration then
                     s_bitcount <= 2;
                     s_duration <= to_signed(1,s_duration'length);
                  end if;
               end if;
            when 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 =>
               if s_duration < s_minduration / 2 then
                  -- nothing
               elsif s_duration = s_minduration / 2 then
                  -- sample halfway
                  s_assembled_byte <= i_rx & s_assembled_byte(7 downto 1);                  
               elsif s_duration = s_minduration or i_rx /= s_last_rx then
                  s_bitcount <= s_bitcount + 1;
                  s_duration <= to_signed(1,s_duration'length);
               end if;
            when 10 => -- stopbit
               if s_duration >= s_minduration / 2 then
                  if s_duration = s_minduration or i_rx = '0' then            
                     s_datain <= s_assembled_byte;
                     s_datain_clk <= '1';
                     s_bitcount <= 0;
                  end if;
               end if;
            end case;
         end case;       
         
         o_datain <= s_datain;
         o_datain_clk <= s_datain_clk;

         -- uncomment next assignments for debugging in testbench
         -- t_state           <= tState'POS(s_state);
         -- t_duration        <= s_duration;
         -- t_iread           <= s_iread;
         -- t_iwrite          <= s_iwrite;
         -- t_count           <= s_count;
         -- t_minduration     <= s_minduration;
         -- t_active_duration <= s_active_duration;
         -- t_bitcount        <= s_bitcount;
         -- t_assembled_byte  <= s_assembled_byte;
      end if;
   end process;   
end architecture UART_arch;


            
            
            
            